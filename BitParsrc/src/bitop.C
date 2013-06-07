
/*******************************************************************/
/*                                                                 */
/*     File: bitop.C                                               */
/*   Author: Helmut Schmid                                         */
/*  Purpose:                                                       */
/*  Created: Fri Feb 13 15:03:55 2004                              */
/* Modified: Thu Sep 20 15:18:21 2012 (schmid)                     */
/*                                                                 */
/*******************************************************************/

#include <assert.h>

#include "bitop.h"


/*******************************************************************/
/*                                                                 */
/*  extract_bits                                                   */
/*                                                                 */
/*******************************************************************/

void extract_bits(size_t b, size_t e, unsigned long *bitvec, vector<size_t> &m)

#ifdef _LP64
{
  assert( b<=e );
  // extraction of the 1 bits from the bit vector
  size_t l = (e - b + ULONG_BITS - 1) / ULONG_BITS;
  size_t bp = b;
  for( size_t i=0; i<l; i++, bp+=ULONG_BITS ) {
    unsigned long n = bitvec[i];
    if (n) {
      if (n & 0x00000000ffffffff) {
	if (n & 0x000000000000ffff) {
	  if (n & 0x00000000000000ff) {
	    if (n & 0x000000000000000f) {
	      if (n & 0x0000000000000003) {
		if (n & 0x0000000000000001) {
		  m.push_back(bp);
		}
		if (n & 0x0000000000000002) {
		  m.push_back(bp + 1);
		}
	      }
	      if (n & 0x000000000000000c) {
		if (n & 0x0000000000000004) {
		  m.push_back(bp + 2);
		}
		if (n & 0x0000000000000008) {
		  m.push_back(bp + 3);
		}
	      }
	    }
	    if (n & 0x00000000000000f0) {
	      if (n & 0x0000000000000030) {
		if (n & 0x0000000000000010) {
		  m.push_back(bp + 4);
		}
		if (n & 0x0000000000000020) {
		  m.push_back(bp + 5);
		}
	      }
	      if (n & 0x00000000000000c0) {
		if (n & 0x0000000000000040) {
		  m.push_back(bp + 6);
		}
		if (n & 0x0000000000000080) {
		  m.push_back(bp + 7);
		}
	      }
	    }
	  }
	  if (n & 0x000000000000ff00) {
	    if (n & 0x0000000000000f00) {
	      if (n & 0x0000000000000300) {
		if (n & 0x0000000000000100) {
		  m.push_back(bp + 8);
		}
		if (n & 0x0000000000000200) {
		  m.push_back(bp + 9);
		}
	      }
	      if (n & 0x0000000000000c00) {
		if (n & 0x0000000000000400) {
		  m.push_back(bp + 10);
		}
		if (n & 0x0000000000000800) {
		  m.push_back(bp + 11);
		}
	      }
	    }
	    if (n & 0x000000000000f000) {
	      if (n & 0x0000000000003000) {
		if (n & 0x0000000000001000) {
		  m.push_back(bp + 12);
		}
		if (n & 0x0000000000002000) {
		  m.push_back(bp + 13);
		}
	      }
	      if (n & 0x000000000000c000) {
		if (n & 0x0000000000004000) {
		  m.push_back(bp + 14);
		}
		if (n & 0x0000000000008000) {
		  m.push_back(bp + 15);
		}
	      }
	    }
	  }
	}
	if (n & 0x00000000ffff0000) {
	  if (n & 0x0000000000ff0000) {
	    if (n & 0x00000000000f0000) {
	      if (n & 0x0000000000030000) {
		if (n & 0x0000000000010000) {
		  m.push_back(bp + 16);
		}
		if (n & 0x0000000000020000) {
		  m.push_back(bp + 17);
		}
	      }
	      if (n & 0x00000000000c0000) {
		if (n & 0x0000000000040000) {
		  m.push_back(bp + 18);
		}
		if (n & 0x0000000000080000) {
		  m.push_back(bp + 19);
		}
	      }
	    }
	    if (n & 0x0000000000f00000) {
	      if (n & 0x0000000000300000) {
		if (n & 0x0000000000100000) {
		  m.push_back(bp + 20);
		}
		if (n & 0x0000000000200000) {
		  m.push_back(bp + 21);
		}
	      }
	      if (n & 0x0000000000c00000) {
		if (n & 0x0000000000400000) {
		  m.push_back(bp + 22);
		}
		if (n & 0x0000000000800000) {
		  m.push_back(bp + 23);
		}
	      }
	    }
	  }
	  if (n & 0x00000000ff000000) {
	    if (n & 0x000000000f000000) {
	      if (n & 0x0000000003000000) {
		if (n & 0x0000000001000000) {
		  m.push_back(bp + 24);
		}
		if (n & 0x0000000002000000) {
		  m.push_back(bp + 25);
		}
	      }
	      if (n & 0x000000000c000000) {
		if (n & 0x0000000004000000) {
		  m.push_back(bp + 26);
		}
		if (n & 0x0000000008000000) {
		  m.push_back(bp + 27);
		}
	      }
	    }
	    if (n & 0x00000000f0000000) {
	      if (n & 0x0000000030000000) {
		if (n & 0x0000000010000000) {
		  m.push_back(bp + 28);
		}
		if (n & 0x0000000020000000) {
		  m.push_back(bp + 29);
		}
	      }
	      if (n & 0x00000000c0000000) {
		if (n & 0x0000000040000000) {
		  m.push_back(bp + 30);
		}
		if (n & 0x0000000080000000) {
		  m.push_back(bp + 31);
		}
	      }
	    }
	  }
	}
      }
      if (n & 0xffffffff00000000) {
	if (n & 0x0000ffff00000000) {
	  if (n & 0x000000ff00000000) {
	    if (n & 0x0000000f00000000) {
	      if (n & 0x0000000300000000) {
		if (n & 0x0000000100000000) {
		  m.push_back(bp + 32);
		}
		if (n & 0x0000000200000000) {
		  m.push_back(bp + 33);
		}
	      }
	      if (n & 0x0000000c00000000) {
		if (n & 0x0000000400000000) {
		  m.push_back(bp + 34);
		}
		if (n & 0x0000000800000000) {
		  m.push_back(bp + 35);
		}
	      }
	    }
	    if (n & 0x000000f000000000) {
	      if (n & 0x0000003000000000) {
		if (n & 0x0000001000000000) {
		  m.push_back(bp + 36);
		}
		if (n & 0x0000002000000000) {
		  m.push_back(bp + 37);
		}
	      }
	      if (n & 0x000000c000000000) {
		if (n & 0x0000004000000000) {
		  m.push_back(bp + 38);
		}
		if (n & 0x0000008000000000) {
		  m.push_back(bp + 39);
		}
	      }
	    }
	  }
	  if (n & 0x0000ff0000000000) {
	    if (n & 0x00000f0000000000) {
	      if (n & 0x0000030000000000) {
		if (n & 0x0000010000000000) {
		  m.push_back(bp + 40);
		}
		if (n & 0x0000020000000000) {
		  m.push_back(bp + 41);
		}
	      }
	      if (n & 0x00000c0000000000) {
		if (n & 0x0000040000000000) {
		  m.push_back(bp + 42);
		}
		if (n & 0x0000080000000000) {
		  m.push_back(bp + 43);
		}
	      }
	    }
	    if (n & 0x0000f00000000000) {
	      if (n & 0x0000300000000000) {
		if (n & 0x0000100000000000) {
		  m.push_back(bp + 44);
		}
		if (n & 0x0000200000000000) {
		  m.push_back(bp + 45);
		}
	      }
	      if (n & 0x0000c00000000000) {
		if (n & 0x0000400000000000) {
		  m.push_back(bp + 46);
		}
		if (n & 0x0000800000000000) {
		  m.push_back(bp + 47);
		}
	      }
	    }
	  }
	}
	if (n & 0xffff000000000000) {
	  if (n & 0x00ff000000000000) {
	    if (n & 0x000f000000000000) {
	      if (n & 0x0003000000000000) {
		if (n & 0x0001000000000000) {
		  m.push_back(bp + 48);
		}
		if (n & 0x0002000000000000) {
		  m.push_back(bp + 49);
		}
	      }
	      if (n & 0x000c000000000000) {
		if (n & 0x0004000000000000) {
		  m.push_back(bp + 50);
		}
		if (n & 0x0008000000000000) {
		  m.push_back(bp + 51);
		}
	      }
	    }
	    if (n & 0x00f0000000000000) {
	      if (n & 0x0030000000000000) {
		if (n & 0x0010000000000000) {
		  m.push_back(bp + 52);
		}
		if (n & 0x0020000000000000) {
		  m.push_back(bp + 53);
		}
	      }
	      if (n & 0x00c0000000000000) {
		if (n & 0x0040000000000000) {
		  m.push_back(bp + 54);
		}
		if (n & 0x0080000000000000) {
		  m.push_back(bp + 55);
		}
	      }
	    }
	  }
	  if (n & 0xff00000000000000) {
	    if (n & 0x0f00000000000000) {
	      if (n & 0x0300000000000000) {
		if (n & 0x0100000000000000) {
		  m.push_back(bp + 56);
		}
		if (n & 0x0200000000000000) {
		  m.push_back(bp + 57);
		}
	      }
	      if (n & 0x0c00000000000000) {
		if (n & 0x0400000000000000) {
		  m.push_back(bp + 58);
		}
		if (n & 0x0800000000000000) {
		  m.push_back(bp + 59);
		}
	      }
	    }
	    if (n & 0xf000000000000000) {
	      if (n & 0x3000000000000000) {
		if (n & 0x1000000000000000) {
		  m.push_back(bp + 60);
		}
		if (n & 0x2000000000000000) {
		  m.push_back(bp + 61);
		}
	      }
	      if (n & 0xc000000000000000) {
		if (n & 0x4000000000000000) {
		  m.push_back(bp + 62);
		}
		if (n & 0x8000000000000000) {
		  m.push_back(bp + 63);
		}
	      }
	    }
	  }
	}
      }
    }
  }
}
#else
{
  assert( b<=e );
  // extraction of the 1 bits from the bit vector
  size_t l = (e - b + ULONG_BITS - 1) / ULONG_BITS;
  size_t bp = b;
  for( size_t i=0; i<l; i++, bp+=ULONG_BITS ) {
    unsigned long n = bitvec[i];
    if (n) {
      if (n & 0x0000ffff) {
	if (n & 0x000000ff) {
	  if (n & 0x0000000f) {
	    if (n & 0x00000003) {
	      if (n & 0x00000001) {
		m.push_back(bp);
	      }
	      if (n & 0x00000002) {
		m.push_back(bp + 1);
	      }
	    }
	    if (n & 0x0000000c) {
	      if (n & 0x00000004) {
		m.push_back(bp + 2);
	      }
	      if (n & 0x00000008) {
		m.push_back(bp + 3);
	      }
	    }
	  }
	  if (n & 0x000000f0) {
	    if (n & 0x00000030) {
	      if (n & 0x00000010) {
		m.push_back(bp + 4);
	      }
	      if (n & 0x00000020) {
		m.push_back(bp + 5);
	      }
	    }
	    if (n & 0x000000c0) {
	      if (n & 0x00000040) {
		m.push_back(bp + 6);
	      }
	      if (n & 0x00000080) {
		m.push_back(bp + 7);
	      }
	    }
	  }
	}
	if (n & 0x0000ff00) {
	  if (n & 0x00000f00) {
	    if (n & 0x00000300) {
	      if (n & 0x00000100) {
		m.push_back(bp + 8);
	      }
	      if (n & 0x00000200) {
		m.push_back(bp + 9);
	      }
	    }
	    if (n & 0x00000c00) {
	      if (n & 0x00000400) {
		m.push_back(bp + 10);
	      }
	      if (n & 0x00000800) {
		m.push_back(bp + 11);
	      }
	    }
	  }
	  if (n & 0x0000f000) {
	    if (n & 0x00003000) {
	      if (n & 0x00001000) {
		m.push_back(bp + 12);
	      }
	      if (n & 0x00002000) {
		m.push_back(bp + 13);
	      }
	    }
	    if (n & 0x0000c000) {
	      if (n & 0x00004000) {
		m.push_back(bp + 14);
	      }
	      if (n & 0x00008000) {
		m.push_back(bp + 15);
	      }
	    }
	  }
	}
      }
      if (n & 0xffff0000) {
	if (n & 0x00ff0000) {
	  if (n & 0x000f0000) {
	    if (n & 0x00030000) {
	      if (n & 0x00010000) {
		m.push_back(bp + 16);
	      }
	      if (n & 0x00020000) {
		m.push_back(bp + 17);
	      }
	    }
	    if (n & 0x000c0000) {
	      if (n & 0x00040000) {
		m.push_back(bp + 18);
	      }
	      if (n & 0x00080000) {
		m.push_back(bp + 19);
	      }
	    }
	  }
	  if (n & 0x00f00000) {
	    if (n & 0x00300000) {
	      if (n & 0x00100000) {
		m.push_back(bp + 20);
	      }
	      if (n & 0x00200000) {
		m.push_back(bp + 21);
	      }
	    }
	    if (n & 0x00c00000) {
	      if (n & 0x00400000) {
		m.push_back(bp + 22);
	      }
	      if (n & 0x00800000) {
		m.push_back(bp + 23);
	      }
	    }
	  }
	}
	if (n & 0xff000000) {
	  if (n & 0x0f000000) {
	    if (n & 0x03000000) {
	      if (n & 0x01000000) {
		m.push_back(bp + 24);
	      }
	      if (n & 0x02000000) {
		m.push_back(bp + 25);
	      }
	    }
	    if (n & 0x0c000000) {
	      if (n & 0x04000000) {
		m.push_back(bp + 26);
	      }
	      if (n & 0x08000000) {
		m.push_back(bp + 27);
	      }
	    }
	  }
	  if (n & 0xf0000000) {
	    if (n & 0x30000000) {
	      if (n & 0x10000000) {
		m.push_back(bp + 28);
	      }
	      if (n & 0x20000000) {
		m.push_back(bp + 29);
	      }
	    }
	    if (n & 0xc0000000) {
	      if (n & 0x40000000) {
		m.push_back(bp + 30);
	      }
	      if (n & 0x80000000) {
		m.push_back(bp + 31);
	      }
	    }
	  }
	}
      }
    }
  }
}
#endif
