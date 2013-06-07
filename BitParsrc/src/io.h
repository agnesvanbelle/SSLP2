
/*******************************************************************/
/*                                                                 */
/*     File: io.h                                                  */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Jun 14 16:35:39 2007                              */
/* Modified: Mon Jan 12 15:34:19 2009 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#ifndef _IO_H
#define _IO_H

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <vector>
using std::vector;

void write_string( const char *p, FILE *file );
char *read_string( FILE *file );
void read_string( char *buffer, FILE *file );
void write_stringvec( const vector<char*> &v, FILE *file );
void read_stringvec( vector<char*> &v, FILE *file );

FILE *open_file( const char *filename, const char *flags );

template <class T> void write_data( const T &a, FILE *file ) {
  fwrite( &a, sizeof(T), 1, file );
  if (ferror(file))
    errx(1, "Error encountered while writing to file");
}

template <class T> void read_data( T &a, FILE *file ) {
  fread( &a, sizeof(T), 1, file );
  if (ferror(file))
    errx(1,  "Error encountered while reading from file");
}

template <class T> void write_datavec( const vector<T> &v, FILE *file ) {
  write_data( v.size(), file );
  if (ferror(file))
    errx(1, "Error encountered while writing to file");
  for( size_t i=0; i<v.size(); i++ )
    v[i].store( file );
}

template <class T> void read_datavec( vector<T> &v, FILE *file ) {
  size_t n;
  read_data( n, file );
  if (ferror(file))
    errx(1, "Error encountered while writing to file");
  v.clear();
  v.reserve(n);
  for( size_t i=0; i<n; i++ )
    v.push_back(T(file));
}

template <class T> void write_basedatavec( const vector<T> &v, FILE *file ) {
  write_data( v.size(), file );
  if (ferror(file))
    errx(1, "Error encountered while writing to file");
  for( size_t i=0; i<v.size(); i++ )
    write_data( v[i], file );
}

template <class T> void read_basedatavec( vector<T> &v, FILE *file ) {
  size_t n;
  read_data( n, file );
  if (ferror(file))
    errx(1, "Error encountered while writing to file");
  v.resize(n);
  for( size_t i=0; i<n; i++ )
    read_data(v[i] , file);
}

#endif
