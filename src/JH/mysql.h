#ifndef __SCR_MYSQL_H__
#define __SCR_MYSQL_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <mysql/mysql.h>

#define SQL_MAX_QUERY_SIZE 1024

struct mysql_async_task
{
    struct mysql_async_task *prev;
    struct mysql_async_task *next;
    int id;
    MYSQL_RES *result;
    bool done;
    bool started;
    bool save;
    char query[SQL_MAX_QUERY_SIZE + 1];
};

struct mysql_async_connection
{
    struct mysql_async_connection *prev;
    struct mysql_async_connection *next;
    struct mysql_async_task* task;
    MYSQL *connection;
};

void JH_mysql_addFunctions();

#ifdef __cplusplus
};
#endif

#endif