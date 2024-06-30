#include "general.h"
#include "mysql.h"

struct mysql_async_connection *first_async_connection = NULL;
struct mysql_async_task *first_async_task = NULL;
MYSQL *cod_mysql_connection = NULL;
pthread_mutex_t lock_async_mysql;

void JH_mysql_async_create_query_nosave();
void JH_mysql_async_create_query();
void JH_mysql_async_getdone_list();
void JH_mysql_async_getresult_and_free();
void JH_mysql_async_initializer();
void JH_mysql_init();
void JH_mysql_reuse_connection();
void JH_mysql_real_connect();
void JH_mysql_error();
void JH_mysql_errno();
void JH_mysql_query();
void JH_mysql_store_result();
void JH_mysql_num_rows();
void JH_mysql_fetch_row();
void JH_mysql_free_result();
void JH_mysql_real_escape_string();
void JH_mysql_close();

void JH_mysql_addFunctions()
{
    Scr_AddFunction("mysql_init", JH_mysql_init, qfalse);
	Scr_AddFunction("mysql_real_connect", JH_mysql_real_connect, qfalse);
	Scr_AddFunction("mysql_query", JH_mysql_query, qfalse);
	Scr_AddFunction("mysql_errno", JH_mysql_errno, qfalse);
	Scr_AddFunction("mysql_error", JH_mysql_error, qfalse);
	Scr_AddFunction("mysql_close", JH_mysql_close, qfalse);
	Scr_AddFunction("mysql_store_result", JH_mysql_store_result, qfalse);
	Scr_AddFunction("mysql_num_rows", JH_mysql_num_rows, qfalse);
	Scr_AddFunction("mysql_fetch_row", JH_mysql_fetch_row, qfalse);
	Scr_AddFunction("mysql_free_result", JH_mysql_free_result, qfalse);
	Scr_AddFunction("mysql_real_escape_string", JH_mysql_real_escape_string, qfalse);
	Scr_AddFunction("mysql_async_create_query", JH_mysql_async_create_query, qfalse);
	Scr_AddFunction("mysql_async_create_query_nosave", JH_mysql_async_create_query_nosave, qfalse);
	Scr_AddFunction("mysql_async_initializer", JH_mysql_async_initializer, qfalse);
	Scr_AddFunction("mysql_async_getdone_list", JH_mysql_async_getdone_list, qfalse);
	Scr_AddFunction("mysql_async_getresult_and_free", JH_mysql_async_getresult_and_free, qfalse);
	Scr_AddFunction("mysql_reuse_connection", JH_mysql_reuse_connection, qfalse);
}

void *mysql_async_execute_query(void *input_c) //cannot be called from gsc, is threaded.
{
    struct mysql_async_connection *c = (struct mysql_async_connection *) input_c;
    int res = mysql_query(c->connection, c->task->query);
    if(!res && c->task->save)
        c->task->result = mysql_store_result(c->connection);
    else if(res)
    {
        //mysql show error here?
    }
    c->task->done = true;
    c->task = NULL;
    return NULL;
}

void *mysql_async_query_handler(void* input_nothing) //is threaded after initialize
{
    static bool started = false;
    if(started)
    {
        printf("async handler already started. Returning\n");
        return NULL;
    }
    started = true;
    struct mysql_async_connection *c = first_async_connection;
    if(c == NULL)
    {
        printf("async handler started before any connection was initialized\n"); //this should never happen
        started = false;
        return NULL;
    }
    struct mysql_async_task *q = NULL;
    while(true)
    {
        pthread_mutex_lock(&lock_async_mysql);
        q = first_async_task;
        c = first_async_connection;
        while(q != NULL)
        {
            if(!q->started)
            {
                while(c != NULL && c->task != NULL)
                    c = c->next;

                if(c == NULL)
                    break; //out of free connections
                q->started = true;
                c->task = q;
                pthread_t query_doer;
                int error = pthread_create(&query_doer, NULL, mysql_async_execute_query, c);
                if(error)
                {
					printf("error: %i\n", error);
                    printf("Error detaching async handler thread\n");
                    pthread_mutex_unlock(&lock_async_mysql);
                    return NULL;
                }
                pthread_detach(query_doer);
                c = c->next;
            }
            q = q->next;
        }
        pthread_mutex_unlock(&lock_async_mysql);
        usleep(10000);
    }
    return NULL;
}

int mysql_async_query_initializer(struct mysql_async_task *newtask, bool save) //cannot be called from gsc, helper function
{
    static int id = 0;
    id++;

    pthread_mutex_lock(&lock_async_mysql);

    struct mysql_async_task *current = first_async_task;
    while((current != NULL) && (current->next != NULL))
        current = current->next;

    newtask->id = id;
    newtask->prev = current;
    newtask->result = NULL;
    newtask->save = save;
    newtask->done = false;
    newtask->next = NULL;
    newtask->started = false;
    if(current != NULL)
        current->next = newtask;
    else
        first_async_task = newtask;
    pthread_mutex_unlock(&lock_async_mysql);
    return id;
}

void JH_mysql_async_create_query_nosave()
{

    struct mysql_async_task *newtask = (struct mysql_async_task*)malloc(sizeof(struct mysql_async_task));
    if(newtask == NULL)
    {
        Scr_AddUndefined();
        return;
    }
    char *query = Scr_GetString(0);

    if(query == NULL)
    {
        Scr_AddUndefined();
        free(newtask);
        return;
    }
    strncpy(newtask->query, query, SQL_MAX_QUERY_SIZE);

	int id = mysql_async_query_initializer(newtask, false);
	Scr_AddInt(id);
}

void JH_mysql_async_create_query()
{

    struct mysql_async_task *newtask = (struct mysql_async_task*)malloc(sizeof(struct mysql_async_task));
    if(newtask == NULL)
    {
        Scr_AddUndefined();
        return;
    }
    char *query = Scr_GetString(0);

    if(query == NULL)
    {
        Scr_AddUndefined();
        free(newtask);
        return;
    }
    strncpy(newtask->query, query, SQL_MAX_QUERY_SIZE);

	int id = mysql_async_query_initializer(newtask, true);
	Scr_AddInt(id);
}

void JH_mysql_async_getdone_list()
{
    pthread_mutex_lock(&lock_async_mysql);
    struct mysql_async_task *current = first_async_task;

    Scr_MakeArray();
    while (current != NULL)
    {
        if (current->done)
        {
            Scr_AddInt((int)current->id);
            Scr_AddArray();
        }
        current = current->next;
    }
    pthread_mutex_unlock(&lock_async_mysql);
}


void JH_mysql_async_getresult_and_free()
{
	int id = Scr_GetInt(0);

    pthread_mutex_lock(&lock_async_mysql);
    struct mysql_async_task *c = first_async_task;
    if (c != NULL)
    {
        while((c != NULL) && (c->id != id))
        {
            c = c->next;
        }
    }
    if (c != NULL)
    {
        if(!c->done)
        {
            Scr_AddUndefined(); //not done yet
            pthread_mutex_unlock(&lock_async_mysql);
            return;
        }
        if(c->next != NULL)
            c->next->prev = c->prev;
        if(c->prev != NULL)
            c->prev->next = c->next;
        else
            first_async_task = c->next;
        if (c->save)
        {
            int ret = (int)c->result;
            Scr_AddInt(ret);
        }
        else
        {
            Scr_AddInt(0);
        }
        free(c);
        pthread_mutex_unlock(&lock_async_mysql);
        return;
    }
    else
    {
        Scr_AddUndefined();
        pthread_mutex_unlock(&lock_async_mysql);
        return;
    }
}

void JH_mysql_async_initializer()
{
    if (first_async_connection != NULL)
    {
		printf("gsc_mysql_async_initializer() async mysql already initialized. Returning before adding additional connections\n");
        return;
    }
    if (pthread_mutex_init(&lock_async_mysql, NULL) != 0)
    {
		printf("Async mutex initialization failed\n");
        return;
    }

	char *host = Scr_GetString(0);
    char *user = Scr_GetString(1);
    char *pass = Scr_GetString(2);
    char *db = Scr_GetString(3);
    int port = Scr_GetInt(4);
    int connection_count= Scr_GetInt(5);

	if(connection_count <= 0)
	{
		printf("gsc_mysql_async_initializer() need a positive connection_count in mysql_async_initializer\n");
		return;
	}

	struct mysql_async_connection *current = first_async_connection;
	for(int i = 0; i < connection_count; i++)
	{
		struct mysql_async_connection *newconnection = (struct mysql_async_connection*)malloc(sizeof(struct mysql_async_connection));
		newconnection->next = NULL;
		newconnection->connection = mysql_init(NULL);
        bool reconnect = true;
		mysql_options(newconnection->connection, MYSQL_OPT_RECONNECT, &reconnect);
		newconnection->connection = mysql_real_connect((MYSQL*)newconnection->connection, host, user, pass, db, port, NULL, 0);
		newconnection->task = NULL;
		if (first_async_connection == NULL)
		{
			newconnection->prev = NULL;
            newconnection->next = NULL;
			first_async_connection = newconnection;
		}
		else
		{
			current->next = newconnection;
			newconnection->prev = current;
            newconnection->next = NULL;
		}
		current = newconnection;
	}
	pthread_t async_handler;
	if (pthread_create(&async_handler, NULL, mysql_async_query_handler, NULL) != 0)
	{
		printf("gsc_mysql_async_initializer() error detaching async handler thread\n");
		return;
	}
	pthread_detach(async_handler);
}

void JH_mysql_init()
{
    MYSQL *connection = mysql_init(NULL);
    if(connection != NULL)
        Scr_AddInt((int)connection);
    else
        Scr_AddUndefined();
}

void JH_mysql_reuse_connection()
{
    if(cod_mysql_connection == NULL)
    {
        Scr_AddUndefined();
        return;
    }
    else
    {
        Scr_AddInt((int) cod_mysql_connection);
        return;
    }
}

void JH_mysql_real_connect()
{
	int input_mysql = Scr_GetInt(0);

	char *host = Scr_GetString(1);
    char *user = Scr_GetString(2);
    char *pass = Scr_GetString(3);
    char *db = Scr_GetString(4);
    int port = Scr_GetInt(5);

	bool reconnect = true;
	mysql_options((MYSQL*)input_mysql, MYSQL_OPT_RECONNECT, &reconnect);
	MYSQL *mysql = mysql_real_connect((MYSQL *)input_mysql, host, user, pass, db, port, NULL, 0);

	if(cod_mysql_connection == NULL)
		cod_mysql_connection = (MYSQL*)mysql;

    if (mysql != NULL)
	    Scr_AddInt((int)mysql);
    else
        Scr_AddUndefined();
}

void JH_mysql_error()
{
	int mysql = Scr_GetInt(0);
	char *ret = (char *)mysql_error((MYSQL *)mysql);
	Scr_AddString(ret);
}

void JH_mysql_errno()
{
	int mysql = Scr_GetInt(0);
	int ret = mysql_errno((MYSQL *)mysql);
	Scr_AddInt(ret);
}

void JH_mysql_close()
{
	int mysql = Scr_GetInt(0);
	mysql_close((MYSQL *)mysql);
}

void JH_mysql_query()
{
    int mysql = Scr_GetInt(0);
    char *query = Scr_GetString(1);
	int ret = mysql_query((MYSQL *)mysql, query);
	Scr_AddInt(ret);
}

void JH_mysql_store_result()
{
	int mysql = Scr_GetInt(0);
	MYSQL_RES *result = mysql_store_result((MYSQL *)mysql);
	Scr_AddInt((int) result);
}

void JH_mysql_num_rows()
{
	int result = Scr_GetInt(0);
	int ret = mysql_num_rows((MYSQL_RES *)result);
	Scr_AddInt(ret);
}

void JH_mysql_fetch_row()
{
	int result = Scr_GetInt(0);
	MYSQL_ROW row = mysql_fetch_row((MYSQL_RES *)result);
	if (!row)
	{
		Scr_AddUndefined();
		return;
	}

	Scr_MakeArray();
	int numfields = mysql_num_fields((MYSQL_RES *)result);
	for (int i = 0; i < numfields; i++)
	{
		if (row[i] == NULL)
        {
			Scr_AddUndefined();
        }
        else
        {
        	Scr_AddString(row[i]);
        }

		Scr_AddArray();
	}
}

void JH_mysql_free_result()
{
    int result = Scr_GetInt(0);

	if (result == 0)
		printf("mysql_free_result() input is a NULL-pointer\n");
	else
        mysql_free_result((MYSQL_RES *)result);

	Scr_AddUndefined();
}

void JH_mysql_real_escape_string()
{
    int mysql = Scr_GetInt(0);
    char *str = Scr_GetString(1);

	char result[SQL_MAX_QUERY_SIZE * 2 + 1];
	mysql_real_escape_string((MYSQL *)mysql, result, str, strlen(str));
	Scr_AddString(result);
}