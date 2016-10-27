//#ifndef _SWAP_ENDIAN
#define _SWAP_ENDIAN

// Macs and SGIs are Big-Endian; PCs are little endian
// returns TRUE if current machine is little endian
extern int IsLittleEndian(void);

/******************************************************************************
  FUNCTION: SwapEndian
  PURPOSE: Swap the byte order of a structure
  EXAMPLE: float F=123.456;; SWAP_FLOAT(F);
******************************************************************************/

void ByteSwap(unsigned char *, int);

