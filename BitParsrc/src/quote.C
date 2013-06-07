
/*MA****************************************************************/
/*                                                                 */
/*     File: quote.C                                               */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Mon Feb  3 09:08:27 2003                              */
/* Modified: Thu Jun  8 13:24:20 2006 (schmid)                     */
/*                                                                 */
/*ME****************************************************************/


/*FA****************************************************************/
/*                                                                 */
/*  quote                                                          */
/*                                                                 */
/*FE****************************************************************/

char *quote( const char *string )

{
  int i,k;
  static char buffer[10000];

  for( i=k=0; k<10000; i++, k++) {
    if (!string[i])
      break;
    if  (string[i] == '"' || string[i] == '\\' || string[i] == ' ' || 
	 string[i] == '$' || string[i] == '^' || string[i] == '\'' ||
	 string[i] == '(' || string[i] == ')' || string[i] == '[' || 
	 string[i] == ']' || string[i] == '{' || string[i] == '}' || 
	 string[i] == '=' || string[i] == '<' || string[i] == '>' ||
	 string[i] == '#')
      buffer[k++] = '\\';
    buffer[k] = string[i];
  }
  buffer[k] = '\0';

  return &buffer[0];
}
