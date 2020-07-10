/***************************************************************
 *                                                             *
 * Dehrust2 from Hrust Library 2 (c) Hrumer.                   *
 * source: packed block without header.                        *
 *                                                             *
 * warning. don't use it for depack files created by hrust2.1. *
 *                                                             *
 ***************************************************************/

#include "dehrust.hpp"

BYTE *hr_d_output;
BYTE *hr_d_input;
BYTE  hr_d_tagbyte;/*буфер для битов.*/

BYTE hr_d_getbit(void)
{
  BYTE tmp;
  
  if(hr_d_tagbyte == 0x80)
  {
    hr_d_tagbyte = *hr_d_input++;
    tmp = hr_d_tagbyte & 0x80;
    (hr_d_tagbyte <<= 1)++;
  }
  else
  {
    tmp = hr_d_tagbyte & 0x80;
    hr_d_tagbyte <<= 1;
  }
  return (tmp >> 7);
}

WORD hr_d_getlen(void)
{
  WORD tmp, result = 1;
  do
  {
    tmp = (hr_d_getbit() << 1) + hr_d_getbit();
    result += tmp;
  } while(tmp == 0x03 && result != 0x10);
  
  return(result);
}

WORD hr_d_getdist(void)
{
  WORD result, tmp;
  
  if(hr_d_getbit())
  {
    result = 0xff00 + *hr_d_input++;
  }
  else
  {
    result = 0xffff;
    tmp    = (hr_d_getbit() << 1) + hr_d_getbit() + 0xffff-3;
    
    for(; tmp; tmp++) result = (result << 1) + hr_d_getbit() - 1;
    if(result == 0xffff - 30)  result = *hr_d_input++;
    result = (result << 8) + *hr_d_input++;
  }
  return (result);
}

WORD dehrust(BYTE* source, BYTE* destination)
{
  BYTE  tmpbuf[6];
  WORD  len;
  short offs;
  
  hr_d_input   = source;
  hr_d_output  = destination;
  hr_d_tagbyte = 0x80;
  
  for(int i = 0; i < 6; i++) tmpbuf[i] = *hr_d_input++;
  
  *hr_d_output++ = *hr_d_input++;
  
  bool done = false;
  while(!done)
  {
    if(!hr_d_getbit())
    {
      len = hr_d_getlen();
      offs = 0xffff;
      
      if(len == 4)
      {
        if(hr_d_getbit())
        {
          len = *hr_d_input++;
          
          if (len == 0)
            done = true;
          else
            if(len < 16) len = (len << 8) + *hr_d_input++;
          
          offs = hr_d_getdist();
          
          for(; len; len--) *hr_d_output = *(hr_d_output++ + offs);
        }
        else
        {
          len = 0;
          for(int i = 4; i; i--) len = (len << 1) + hr_d_getbit();
          len += 6;
          len <<= 1;
          for(; len; len--) *hr_d_output++ = *hr_d_input++;
        }
      }
      else
      {
        if(len > 4) len--;
        if(len == 1)
        {
          for(int i = 3; i; i--) offs = (offs << 1) + hr_d_getbit();
        }
        else
        {
          if(len == 2)
            offs = 0xff00 + *hr_d_input++;
          else
            offs = hr_d_getdist();
        }
        for(; len; len--) *hr_d_output = *(hr_d_output++ + offs);
      }
    }
    else
    {
      *hr_d_output++ = *hr_d_input++;
    }
  }
  
  for(int i = 0; i < 6; i++) *hr_d_output++ = tmpbuf[i];
  
  return (hr_d_output - destination);
}

WORD crc16calc(BYTE* adr, WORD len)
{
  WORD crc = 0;
  
  for(; len; len--)
  {
    crc ^= ((*adr) << 8);
    for(int tmp = 8; tmp; tmp--)
      if(crc & 0x8000)
      {
        crc <<= 1;
        crc ^= 0x1021;
      }
      else
      {
        crc <<= 1;
      }
    ++adr;
  }
  return (crc);
}
