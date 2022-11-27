#ifndef _QUERIES_HPP
#define _QUERIES_HPP

#include <cstdio>
#include <stdlib.h>
#include <mutex>

#include "db.hpp"

// execute_* //////////////////////////////////////////////////////////////////
void execute_select(std::string *fout, database_t* const db, const char* const field,
                    const char* const value);
void execute_select(std::string *fout, database_t* const db, const char* const field,
                    const char* const value);

void execute_update(std::string *fout, database_t* const db, const char* const ffield,
                    const char* const fvalue, const char* const efield, const char* const evalue);

void execute_insert(std::string *fout, database_t* const db, const char* const fname,
                    const char* const lname, const unsigned id, const char* const section,
                    const tm birthdate);

void execute_dump(std::string *fout, database_t* const db);

// parse_and_execute_* ////////////////////////////////////////////////////////
void parse_and_execute_select(std::string *fout, database_t* db, const char* const query,std::mutex *read, std::mutex *write, std::mutex *general, int *rQ);

void parse_and_execute(std::string *fout, database_t* db, const char* const query, std::mutex* read, std::mutex* write, std::mutex* general, int* rQ);
<<<<<<< HEAD
void parse_and_execute_update(std::string *fout, database_t* db, const char* const query, std::mutex* write, std::mutex* general);
=======

void parse_and_execute_update(std::string *fout, database_t* db, const char* const query,std::mutex* read, std::mutex* write, std::mutex* general, int* rQ);
>>>>>>> db7bea6 (Modif on queries)

void parse_and_execute_insert(std::string *fout, database_t* db, const char* const query, std::mutex* write, std::mutex* general);

void parse_and_execute_delete(std::string *fout, database_t* db, const char* const query, std::mutex* write, std::mutex* general);

// query_fail_* ///////////////////////////////////////////////////////////////

/** Those methods write a descriptive error message on fout */

void query_fail_bad_query_type(std::string *const fout);

void query_fail_bad_format(std::string *const fout, const char* const query_type);

void query_fail_too_long(std::string *const fout, const char* const query_type);

void query_fail_bad_filter(std::string *const fout, const char* const field, const char* const filter);

void query_fail_bad_update(std::string *const fout, const char* const field, const char* const filter);

// mutex function///////////////////////////////////////////////////////////////

void begin_lock(std::mutex* write, std::mutex* general);

#endif
