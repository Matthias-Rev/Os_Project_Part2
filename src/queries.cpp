#include "queries.hpp"
#include "student.hpp"
#include <string>
#include <iostream>
using namespace std;

// execute_* ///////////////////////////////////////////////////////////////////

void execute_select(string *fout, database_t* const db, const char* const field,
                    const char* const value) {
  string found_w = "";
  int count = 0;
  size_t buffersize = 428;
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return;
  }
  for (const student_t& s : db->data) {
    if (predicate(s)) { 
      count++;
      char buffer[428];
      student_to_str(buffer, &s, buffersize);
      found_w = found_w + (string)buffer + "\n";
    }
  }
  string final_rep = " student(s) selected";
  found_w = found_w + to_string(count) + final_rep;
  *fout = found_w;
}

void execute_update(string *fout, database_t* const db, const char* const ffield, const char* const fvalue, const char* const efield, const char* const evalue) {
  std::function<bool(const student_t&)> predicate = get_filter(ffield, fvalue);
  int count = 0;
  if (!predicate) {
    query_fail_bad_filter(fout, ffield, fvalue);
    return;
  }
  std::function<void(student_t&)> updater = get_updater(efield, evalue);
  if (!updater) {
    query_fail_bad_update(fout, efield, evalue);
    return;
  }
  for (student_t& s : db->data) {
    if (predicate(s)) {
      updater(s);
      count++;
    }
  }
  string final_rep = " student(s) updated";
  *fout = to_string(count) +final_rep;
}

void execute_insert(string *fout, database_t* const db, const char* const fname,
                    const char* const lname, const unsigned id, const char* const section,
                    const tm birthdate) {
  db->data.emplace_back();
  student_t *s = &db->data.back();
  char buffer[428];
  s->id = id;
  snprintf(s->fname, sizeof(s->fname), "%s", fname);
  snprintf(s->lname, sizeof(s->lname), "%s", lname);
  snprintf(s->section, sizeof(s->section), "%s", section);
  s->birthdate = birthdate;
  student_to_str(buffer, s, 428);
  cout << buffer;
  *fout = *fout + buffer;
}

void execute_delete(string *fout, database_t* const db, const char* const field,
                    const char* const value) {
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  int begin = db->data.size();
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return;
  }
  auto new_end = remove_if(db->data.begin(), db->data.end(), predicate);
  db->data.erase(new_end, db->data.end());
  int end = begin - db->data.size();
  string final_rep = " student(s) deleted";
  *fout = to_string(end) +final_rep;
}

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(string *fout, database_t* db, const char* const query,mutex *read, mutex *write, mutex *general, int *rQ) {
  char ffield[32], fvalue[64];  // filter data
  int  counter;
  if (sscanf(query, "select %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "select");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "select");
  } else {
    general->lock();          //permet de lock le mutex géneral
    read->lock();             //permet de lock le read
    if (*rQ == 0){            // nous permet d'éviter la famine du à plusieur écriture
      write->lock();          //si y a aucune lecture qui a été faite recemment on bloque les écritures
    }
    *rQ = *rQ + 1;            // on ajoute une lecture au int
    general->unlock();        // on unlock le general
    read->unlock();           // on unlock la lecture pour permettre à un autre read de lire la db
    execute_select(fout, db, ffield, fvalue);
    read->lock();             // on lock la lecture pour permettre de soustraire le int
    *rQ = *rQ - 1;
    if (*rQ == 0){            //est ce qu'il reste une lecture ?
      write->unlock();        // si il en reste plus alors on debloque l'écriture
    }
    read->unlock();           //on rend la main sur la lecture
  }
}

void parse_and_execute_update(string *fout, database_t* db, const char* const query, mutex *write, mutex *general) {
  char ffield[32], fvalue[64];  // filter data
  char efield[32], evalue[64];  // edit data
  int counter;
  if (sscanf(query, "update %31[^=]=%63s set %31[^=]=%63s%n", ffield, fvalue, efield, evalue,
             &counter) != 4) {
    query_fail_bad_format(fout, "update");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "update");
  } else {
    begin_lock(write, general);
    execute_update(fout, db, ffield, fvalue, efield, evalue);
    write->unlock();
  }
}

void parse_and_execute_insert(string *fout, database_t* db, const char* const query, mutex *write, mutex *general) {
  char      fname[64], lname[64], section[64], date[11];
  unsigned  id;
  tm        birthdate;
  int       counter;
  if (sscanf(query, "insert %63s %63s %u %63s %10s%n", fname, lname, &id, section, date, &counter) != 5 || strptime(date, "%d/%m/%Y", &birthdate) == NULL) {
    query_fail_bad_format(fout, "insert");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "insert");
  } else {
    begin_lock(write, general);
    execute_insert(fout, db, fname, lname, id, section, birthdate);
    write->unlock();
  }
}

void parse_and_execute_delete(string *fout, database_t* db, const char* const query, mutex *write, mutex *general) {
  char ffield[32], fvalue[64]; // filter data
  int counter;
  if (sscanf(query, "delete %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "delete");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "delete");
  } else {
    begin_lock(write, general);
    execute_delete(fout, db, ffield, fvalue);
    write->unlock();
  }
}

void parse_and_execute(string *fout, database_t* db, const char* const query, mutex* read, mutex* write, mutex* general, int* rQ) {
  //void parse_and_execute(FILE* fout, database_t* db, const char* const query) {
  if (strncmp("select", query, sizeof("select")-1) == 0) {
    parse_and_execute_select(fout, db, query,read, write, general, rQ);
  } else if (strncmp("update", query, sizeof("update")-1) == 0) {
    parse_and_execute_update(fout, db, query, write, general);
  } else if (strncmp("insert", query, sizeof("insert")-1) == 0) {
    parse_and_execute_insert(fout, db, query, write, general);
  } else if (strncmp("delete", query, sizeof("delete")-1) == 0) {
    parse_and_execute_delete(fout, db, query, write, general);
  }
}

// query_fail_* ///////////////////////////////////////////////////////////////

void query_fail_bad_query_type(string *fout) {
  string answ="Unknown query type";
  *fout= answ;
}
void query_fail_bad_format(string *fout, const char * const query_type) {
  string answ="Syntax error in up " + (string)query_type;
  *fout= answ;
}
void query_fail_too_long(string *fout, const char * const query_type) {
  string answ="The " + (string)query_type +"has to many char. !";
  *fout= answ;
}
void query_fail_bad_filter(string *fout, const char* const field, const char* const filter) {
  string answ="Syntax error: this field doesn't exist " + (string)field;
  *fout= answ;
}
void query_fail_bad_update(string *fout, const char* const field, const char* const filter) {
  printf("%s and %s are bad filter", field, filter);
  string answ="You can not apply ";
  *fout= answ;
}

void begin_lock(mutex* write, mutex* general){
  general->lock();
  write->lock();
  general->unlock();
}

