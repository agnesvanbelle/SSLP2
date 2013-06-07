
/*******************************************************************/
/*                                                                 */
/*     File: sgi.h                                                 */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Thu Sep 11 15:58:25 2008                              */
/* Modified: Thu Sep 11 15:59:23 2008 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#ifndef _SGI_INCLUDED
#define _SGI_INCLUDED

#ifdef SGI__gnu_cxx

#include <ext/hash_map>
#include <ext/hash_set>
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;
using __gnu_cxx::hash;

#else

#ifdef SGIext
#include <ext/hash_map>
#include <ext/hash_set>
#else
#include <backward/hash_map>
#include <backward/hash_set>
#endif

using std::hash_map;
using std::hash_set;
using std::hash;

#endif

#endif
