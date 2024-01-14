/* 7zStream.c -- 7z Stream functions
   2010-03-11 : Igor Pavlov : Public domain */

#include <stdint.h>
#include <stdint.h>
#include <string.h>

#include "7zTypes.h"

SRes SeqInStream_Readuint8_t(ISeqInStream *stream, uint8_t *buf)
{
   size_t processed = 1;
   int result       = stream->Read(stream, buf, &processed);
   if (result != 0)
      return result;
   if (processed != 1)
      return SZ_ERROR_INPUT_EOF;
   return SZ_OK;
}

SRes SeqInStream_Read2(ISeqInStream *stream, void *buf, size_t size, SRes errorType)
{
   while (size != 0)
   {
      size_t processed = size;
      int result       = stream->Read(stream, buf, &processed);
      if (result    != 0)
         return result;
      if (processed == 0)
         return errorType;
      buf              = (void *)((uint8_t *)buf + processed);
      size            -= processed;
   }
   return SZ_OK;
}

SRes SeqInStream_Read(ISeqInStream *stream, void *buf, size_t size)
{
   return SeqInStream_Read2(stream, buf, size, SZ_ERROR_INPUT_EOF);
}


SRes LookInStream_SeekTo(ILookInStream *stream, uint64_t offset)
{
   int64_t t = offset;
   return stream->Seek(stream, &t, SZ_SEEK_SET);
}

SRes LookInStream_LookRead(ILookInStream *stream, void *buf, size_t *size)
{
   int result;
   const void *lookBuf;
   if (*size == 0)
      return SZ_OK;
   result = stream->Look(stream, &lookBuf, size);
   if (result != 0)
      return result;
   memcpy(buf, lookBuf, *size);
   return stream->Skip(stream, *size);
}

SRes LookInStream_Read2(ILookInStream *stream,
      void *buf, size_t size, SRes errorType)
{
   while (size != 0)
   {
      size_t processed = size;
      int result       = stream->Read(stream, buf, &processed);
      if (result    != 0)
         return result;
      if (processed == 0)
         return errorType;
      buf              = (void *)((uint8_t *)buf + processed);
      size            -= processed;
   }
   return SZ_OK;
}

SRes LookInStream_Read(ILookInStream *stream, void *buf, size_t size)
{
   return LookInStream_Read2(stream, buf, size, SZ_ERROR_INPUT_EOF);
}

static SRes LookToRead_Look_Lookahead(void *pp, const void **buf, size_t *size)
{
   SRes res = SZ_OK;
   CLookToRead *p = (CLookToRead *)pp;
   size_t size2 = p->size - p->pos;
   if (size2 == 0 && *size > 0)
   {
      p->pos = 0;
      size2 = LookToRead_BUF_SIZE;
      res = p->realStream->Read(p->realStream, p->buf, &size2);
      p->size = size2;
   }
   if (size2 < *size)
      *size = size2;
   *buf = p->buf + p->pos;
   return res;
}

static SRes LookToRead_Look_Exact(void *pp, const void **buf, size_t *size)
{
   SRes res = SZ_OK;
   CLookToRead *p = (CLookToRead *)pp;
   size_t size2 = p->size - p->pos;
   if (size2 == 0 && *size > 0)
   {
      p->pos = 0;
      if (*size > LookToRead_BUF_SIZE)
         *size = LookToRead_BUF_SIZE;
      res = p->realStream->Read(p->realStream, p->buf, size);
      size2 = p->size = *size;
   }
   if (size2 < *size)
      *size = size2;
   *buf = p->buf + p->pos;
   return res;
}

static SRes LookToRead_Skip(void *pp, size_t offset)
{
   CLookToRead *p = (CLookToRead *)pp;
   p->pos += offset;
   return SZ_OK;
}

static SRes LookToRead_Read(void *pp, void *buf, size_t *size)
{
   CLookToRead *p = (CLookToRead *)pp;
   size_t rem = p->size - p->pos;
   if (rem == 0)
      return p->realStream->Read(p->realStream, buf, size);
   if (rem > *size)
      rem = *size;
   memcpy(buf, p->buf + p->pos, rem);
   p->pos += rem;
   *size = rem;
   return SZ_OK;
}

static SRes LookToRead_Seek(void *pp, int64_t *pos, ESzSeek origin)
{
   CLookToRead *p = (CLookToRead *)pp;
   p->pos = p->size = 0;
   return p->realStream->Seek(p->realStream, pos, origin);
}

void LookToRead_CreateVTable(CLookToRead *p, int lookahead)
{
   p->s.Look = lookahead ?
      LookToRead_Look_Lookahead :
      LookToRead_Look_Exact;
   p->s.Skip = LookToRead_Skip;
   p->s.Read = LookToRead_Read;
   p->s.Seek = LookToRead_Seek;
}

void LookToRead_Init(CLookToRead *p)
{
   p->pos = p->size = 0;
}

static SRes SecToLook_Read(void *pp, void *buf, size_t *size)
{
   CSecToLook *p = (CSecToLook *)pp;
   return LookInStream_LookRead(p->realStream, buf, size);
}

void SecToLook_CreateVTable(CSecToLook *p)
{
   p->s.Read = SecToLook_Read;
}

static SRes SecToRead_Read(void *pp, void *buf, size_t *size)
{
   CSecToRead *p = (CSecToRead *)pp;
   return p->realStream->Read(p->realStream, buf, size);
}

void SecToRead_CreateVTable(CSecToRead *p)
{
   p->s.Read = SecToRead_Read;
}
