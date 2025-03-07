//
// Unpacking FEAM data
// K.Olchanski
//

#include "Feam.h"

#include <stdio.h> // NULL
#include <stdlib.h> // realloc()
#include <string.h> // memcpy()
#include <assert.h> // assert()
#include <utility>  // std::pair
#include <iostream>

#if 0
static uint8_t getUint8(const void* ptr, int offset)
{
   return *(uint8_t*)(((char*)ptr)+offset);
}

static uint16_t getUint16be(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return ((ptr8[0]<<8) | ptr8[1]);
}

static uint32_t getUint32be(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return (ptr8[0]<<24) | (ptr8[1]<<16) | (ptr8[2]<<8) | ptr8[3];
}
#endif

static uint16_t getUint16le(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return ((ptr8[1]<<8) | ptr8[0]);
}

static uint32_t getUint32le(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return (ptr8[3]<<24) | (ptr8[2]<<16) | (ptr8[1]<<8) | ptr8[0];
}

PwbPadMap::PwbPadMap()
{
   //const int first = 0;  // change if index is numbered from 1
   for(int i = 0; i < MAX_FEAM_READOUT; i++){
      if(i == 1) channel[i] = PWB_CHAN_RESET1;
      else if(i == 2) channel[i] = PWB_CHAN_RESET2;
      else if(i == 3) channel[i] = PWB_CHAN_RESET3;
      else if(i == 16) channel[i] = PWB_CHAN_FPN1;
      else if(i == 29) channel[i] = PWB_CHAN_FPN2;
      else if(i == 54) channel[i] = PWB_CHAN_FPN3;
      else if(i == 67) channel[i] = PWB_CHAN_FPN4;
      else {
         int ch = i - int(i > 16) - int(i > 29) - int(i > 54) - int(i > 67) - 3;
         channel[i] = ch;
         readout[ch] = i;
      }
   }
   for(int isca = 0; isca < MAX_FEAM_SCA; isca++){
      padcol[isca][0] = -99;
      padrow[isca][0] = -99;
   }
   for(int isca = 0; isca < MAX_FEAM_SCA; isca++){
      int offset = (isca%2)*MAX_FEAM_PAD_ROWS/2;
      for(int ch = 1; ch <= MAX_FEAM_CHAN; ch++){
         int col, row;
         if(ch <= 36){
            if(ch > 18){
               col = 0;
               row = 36-ch+offset;
            } else {
               col = 1;
               row = ch-1+offset;
            }
         } else {
            if(ch < 55){
               col = 0;
               row = 72-ch+offset;
            } else {
               col = 1;
               row = ch-37+offset;
            }
         }
         if(isca > 1){
            col = 3-col;
            row = 71-row;
         }
         padcol[isca][ch] = col;
         padrow[isca][ch] = row;
         sca[col][row] = isca;
         sca_chan[col][row] = ch;
      }
   }
};

bool PwbPadMap::CheckMap() const
{
   // check pad map for consistency
   
   printf("Pad map:\n");
   printf("  sca chan: ");
   for (int i=0; i<=79; i++)
      printf("%d ", channel[i]);
   printf("\n");
   for (int sca=0; sca<4; sca++) {
      printf("sca %d:\n", sca);
      printf("  tpc col: ");
      for (int i=0; i<=72; i++)
         printf("%d ", padcol[sca][i]);
      printf("\n");
      printf("  tpc row: ");
      for (int i=0; i<=72; i++)
         printf("%d ", padrow[sca][i]);
      printf("\n");
   }
   
   int test[4*4*18];
   for (int i=0; i<4*4*18; i++)
      test[i] = 0;
   
   bool map_ok = true;
   
   for (int sca=0; sca<4; sca++) {
      for (int i=0; i<=79; i++) {
         int seqsca = sca*80+i;
         int chan = channel[i];
         if (chan > 0) {
            int col = padcol[sca][chan];
            int row = padrow[sca][chan];
            int seqpad = col*4*18+row;
            //hpadmap->Fill(seqpad, seqsca);
            if (test[seqpad] != 0) {
               printf("pad map error: col %d, row %d, seqpad %d: duplicate mapping seqsca %d and %d\n", col, row, seqpad, seqsca, test[seqpad]);
               map_ok = false;
            } else {
               test[seqpad] = seqsca;
            }
         }
      }
   }
   
   for (int i=0; i<4*4*18; i++) {
      if (test[i] == 0) {
         printf("pad map error: seqpad %d is not mapped to sca channel!\n", i);
         map_ok = false;
      }
   }
   
   if (map_ok) {
      printf("pad map is ok.\n");
   } else {
      printf("pad map has errors!\n");
   }

   return map_ok;
}

static PwbPadMap* gMap = NULL;

const PwbPadMap* PwbPadMap::Map()
{
   if (!gMap) {
      gMap = new PwbPadMap();
      bool pwb_map_is_ok = gMap->CheckMap();
      assert(pwb_map_is_ok);
   }

   return gMap;
}

bool PwbPadMap::chan_is_pad(int chan)
{
   return (chan > 0);
}

bool PwbPadMap::chan_is_fpn(int chan)
{
   return (chan >= -4) && (chan <= -1);
}

bool PwbPadMap::chan_is_reset(int chan)
{
   return (chan >= -7) && (chan <= -5);
}

//static int x1count = 0;

FeamPacket::FeamPacket()
{
   //printf("FeamPacket: ctor!\n");
   //printf("FeamPacket: count %d!\n", x1count++);
   error = true;
}

FeamPacket::~FeamPacket()
{
   //printf("FeamPacket: dtor!\n");
   //x1count--;
}

void FeamPacket::Unpack(const char* data, int size)
{
   error = true;

   off = 0;
   cnt = getUint32le(data, off); off += 4;
   n   = getUint16le(data, off); off += 2;
   x511 = getUint16le(data, off); off += 2;
   buf_len = getUint16le(data, off); off += 2;
   if (n == 0) {
      ts_start = getUint32le(data, off); off += 8;
      ts_trig  = getUint32le(data, off); off += 8;
   } else {
      ts_start = 0;
      ts_trig  = 0;
   }

   error = false;
}

void FeamPacket::Print() const
{
   printf("decoded %2d bytes, ", off);
   printf("cnt %6d, n %3d, x511 %3d, buf_len %4d, ts_start 0x%08x, ts_trig 0x%08x, ",
          cnt,
          n,
          x511,
          buf_len,
          ts_start,
          ts_trig);
   printf("error %d", error);
}

//static int x2count = 0;

void FeamModuleData::Finalize()
{
   if (error) {
      return;
   }

   if (fSize != 310688) {
      error = true;
      return;
   }
}

/*
   ZZZ Run 421
   ZZZ Processing FEAM event: module  4, cnt     84, ts_start 0xc8f30f8a, ts_trig 0xc8f311fa, next_n 256, size 310688, error 0
   ZZZ Processing FEAM event: module  5, cnt     97, ts_start 0xc8f6d5d4, ts_trig 0xc8f6d844, next_n 256, size 310688, error 0
   ZZZ Processing FEAM event: module  6, cnt     93, ts_start 0xc8f91bb4, ts_trig 0xc8f91e24, next_n 256, size 310688, error 0
   ZZZ Processing FEAM event: module  7, cnt     83, ts_start 0xc8f5c6dc, ts_trig 0xc8f5c94c, next_n 256, size 310688, error 0
   ZZZ Processing FEAM event: module  0, cnt     80, ts_start 0xc8f46ecc, ts_trig 0xc8f4713c, next_n 256, size 310688, error 0
   ZZZ Processing FEAM event: module  1, cnt   1465, ts_start 0xc8f2ce84, ts_trig 0xc8f2d0f4, next_n 256, size 310688, error 0
   ZZZ Processing FEAM event: module  2, cnt     63, ts_start 0xc8f2892c, ts_trig 0xc8f28b9c, next_n 256, size 310688, error 0
   ZZZ Processing FEAM event: module  3, cnt     62, ts_start 0xc8ef4a56, ts_trig 0xc8ef4cc6, next_n 256, size 310688, error 0
*/

void FeamModuleData::Print(int level) const
{
   printf("pos %2d, ", fPosition);
   printf("pwb%02d, ", fModule);
   printf("c%dr%d, ", fColumn, fRing);
   printf("fmt %d, ", fDataFormat);
   printf("cnt %6d, ts_start 0x%08x, ts_trig 0x%08x, ",
          cnt,
          ts_start,
          ts_trig);
   printf("size %d, ", fSize);
   printf("error %d", error);

   if (level > 0) {
      printf("\n");
      printf("ADC data:\n");
      const uint16_t* aptr16 = (uint16_t*)fPtr;
      int asize16 = fSize/2;
      for (int i=0; i<asize16; i++) {
         if (i%4 == 0)
            printf("%d: ", i);
         printf(" 0x%04x", aptr16[i]);
         if (i%4 == 3)
            printf("\n");
      }
      printf("ADC data done\n");
   }
}

int FeamModuleData::fgMaxAlloc = 0;

void FeamModuleData::AddData(const FeamPacket*p, const char* ptr, int size)
{
   //printf("add %d size %d\n", p->n, size);

   assert(size >= 0);
   assert(size < 12000); // UDP packet size is 1500 bytes, jumbo frame up to 9000 bytes

   int new_size = fSize + size;

   if (new_size > fAlloc) {
      if (new_size < fgMaxAlloc)
         new_size = fgMaxAlloc;
      //printf("realloc %d -> %d, max %d\n", fAlloc, new_size, fgMaxAlloc);
      char* new_ptr = (char*)realloc(fPtr, new_size);

      if (!new_ptr) {
         printf("FeamModuleData::AddData: cannot reallocate ADC buffer from %d to %d bytes!\n", fSize, new_size);
         error = true;
         return;
      }

      fAlloc = new_size;
      fPtr = new_ptr;

      if (fAlloc > fgMaxAlloc)
         fgMaxAlloc = fAlloc;
   }

   memcpy(fPtr + fSize, ptr, size);
   fSize += size;
}

FeamModuleData::FeamModuleData(const FeamPacket* p, int position, int imodule, int icolumn, int iring, int format)
{
   //printf("FeamModuleData: ctor! %d\n", x2count++);
   assert(p->n == 0);

   fPosition = position;
   fModule = imodule;
   fColumn = icolumn;
   fRing = iring;
   fDataFormat = format;

   cnt = p->cnt;
   ts_start = p->ts_start;
   ts_trig = p->ts_trig;
}

FeamModuleData::~FeamModuleData() // dtor
{
   //printf("FeamModuleData: dtor!\n"); x2count--;
   if (fPacket) {
      delete fPacket;
      fPacket = NULL;
   }
   if (fPtr)
      free(fPtr);
   fPtr = NULL;
   fSize = 0;
   fAlloc = 0;
}

#define ST_INIT  0
#define ST_DATA  1
#define ST_WAIT  2
#define ST_DONE  3

FeamModuleAsm::~FeamModuleAsm()
{
   fState = -1;
   fCnt = 0;
   fNextN = -1;
   if (fCurrent) {
      delete fCurrent;
      fCurrent = NULL;
   }
   for (unsigned i=0; i<fBuffer.size(); i++) {
      if (fBuffer[i]) {
         delete fBuffer[i];
         fBuffer[i] = NULL;
      }
   }
}

void FeamModuleAsm::Print() const
{
   int countComplete = 0;
   int countError = 0;
   for (unsigned i=0; i<fBuffer.size(); i++) {
      if (fBuffer[i]->complete)
         countComplete++;
      if (fBuffer[i]->error)
         countError++;
   }
   printf("pwb%02d, state %d, cnt %d, nextn %d, ig %d, fi %d, do %d (sy %d, tr %d, sk %d, wcnt %d), cur %p, buf %d (com %d, err %d)", fModule, fState, fCnt, fNextN, fCountIgnoredBeforeFirst, fCountFirst, fCountDone, fCountLostSync, fCountTruncated, fCountSkip, fCountWrongCnt, fCurrent, (int)fBuffer.size(), countComplete, countError);
}

void FeamModuleAsm::StFirstPacket(const FeamPacket* p, int position, int imodule, int icolumn, int iring, int format, const char* ptr, int size)
{
   fState = ST_DATA;
   fCnt = p->cnt;
   fNextN = 1;
   fCountFirst++;

   fDataFormat = format;
   fModule = imodule;

   assert(fCurrent == NULL);

   fCurrent = new FeamModuleData(p, position, imodule, icolumn, iring, format);
   fCurrent->fPacket = new FeamPacket(*p); // copy constructor

   fCurrent->AddData(p, ptr, size);
}

void FeamModuleAsm::StLastPacket()
{
   fState = ST_DONE;
   fCountDone++;

   assert(fCurrent != NULL);

   fCurrent->complete = true;
   fBuffer.push_back(fCurrent);
   fCurrent = NULL;
}

void FeamModuleAsm::AddData(const FeamPacket* p, const char* ptr, int size)
{
   assert(fCurrent != NULL);
   fCurrent->AddData(p, ptr, size);
}

void FeamModuleAsm::FlushIncomplete()
{
   assert(fCurrent != NULL);

   fCurrent->complete = false;
   fCurrent->error = true;
   fBuffer.push_back(fCurrent);
   fCurrent = NULL;
}

void FeamModuleAsm::AddPacket(const FeamPacket* p, int position, int imodule, int icolumn, int iring, int format, const char* ptr, int size)
{
   bool trace = false;
   bool traceNormal = false;
   
   switch (fState) {
   default: {
      assert(!"invalid state!");
      break;
   }
   case ST_INIT: { // initial state, before received first first packet
      if (p->n==0) { // first packet
         if (trace)
            printf("ST_INIT: pwb%02d, packet cnt %d, n %d ---> first packet\n", imodule, p->cnt, p->n);
         StFirstPacket(p, position, imodule, icolumn, iring, format, ptr, size);
      } else {
         if (traceNormal)
            printf("ST_INIT: pwb%02d, packet cnt %d, n %d\n", imodule, p->cnt, p->n);
         fCountIgnoredBeforeFirst++;
      }
      break;
   }
   case ST_DATA: { // receiving data
      //printf("ST_FIRST: bank %s, packet cnt %d, n %d\n", bank, p->cnt, p->n);
      if (p->n == 0) { // unexpected first packet
         if (trace)
            printf("ST_DATA: pwb%02d, packet cnt %d, n %d ---> unexpected first packet\n", imodule, p->cnt, p->n);
         fCountTruncated++;
         FlushIncomplete();
         StFirstPacket(p, position, imodule, icolumn, iring, format, ptr, size);
      } else if (p->cnt != fCnt) { // packet from wrong event
         if (trace)
            printf("ST_DATA: pwb%02d, packet cnt %d, n %d ---> wrong cnt expected cnt %d\n", imodule, p->cnt, p->n, fCnt);
         fState = ST_WAIT;
         FlushIncomplete();
         fCountWrongCnt++;
      } else if (p->n != fNextN) { // out of sequence packet
         if (trace)
            printf("ST_DATA: pwb%02d, packet cnt %d, n %d ---> out of sequence expected n %d\n", imodule, p->cnt, p->n, fNextN);
         fState = ST_WAIT;
         FlushIncomplete();
         fCountLostSync++;
      } else if (p->n == 255) { // last packet
         if (trace)
            printf("ST_DATA: pwb%02d, packet cnt %d, n %d ---> last packet\n", imodule, p->cnt, p->n);
         AddData(p, ptr, size);
         fNextN++;
         StLastPacket();
      } else {
         if (traceNormal)
            printf("ST_DATA: pwb%02d, packet cnt %d, n %d\n", imodule, p->cnt, p->n);
         AddData(p, ptr, size);
         fNextN++;
      }
      break;
   }
   case ST_WAIT: { // skipping bad data
      if (p->n == 0) { // first packet
         if (trace)
            printf("ST_WAIT: pwb%02d, packet cnt %d, n %d ---> first packet\n", imodule, p->cnt, p->n);
         StFirstPacket(p, position, imodule, icolumn, iring, format, ptr, size);
      } else {
         if (traceNormal)
            printf("ST_WAIT: pwb%02d, packet cnt %d, n %d\n", imodule, p->cnt, p->n);
         fCountSkip++;
      }
      break;
   }
   case ST_DONE: { // received last packet
      if (p->n == 0) { // first packet
         if (trace)
            printf("ST_DONE: pwb%02d, packet cnt %d, n %d ---> first packet\n", imodule, p->cnt, p->n);
         StFirstPacket(p, position, imodule, icolumn, iring, format, ptr, size);
      } else {
         if (trace)
            printf("ST_DONE: pwb%02d, packet cnt %d, n %d ---> lost first packet\n", imodule, p->cnt, p->n);
         fState = ST_WAIT;
         fCountLostFirst++;
      }
      break;
   }
   } // switch (fState)
}

void FeamModuleAsm::Finalize()
{
   switch (fState) {
   default: {
      assert(!"invalid state!");
      break;
   }
   case ST_INIT: { // initial state, before received first first packet
      break;
   }
   case ST_DATA: { // receiving data
      FlushIncomplete();
      fState = ST_DONE;
      break;
   }
   case ST_WAIT: { // skipping bad data
      break;
   }
   case ST_DONE: { // received last packet
      break;
   }
   } // switch (fState)
}

FeamEvent::FeamEvent() // ctor
{
   complete = false;
   error = false;
   counter = 0;
   time = 0;
   timeIncr = 0;
}

FeamEvent::~FeamEvent() // dtor
{
   for (unsigned i=0; i<modules.size(); i++)
      if (modules[i]) {
         delete modules[i];
         modules[i] = NULL;
      }

   for (unsigned i=0; i<adcs.size(); i++)
      if (adcs[i]) {
         delete adcs[i];
         adcs[i] = NULL;
      }

   for (unsigned i=0; i<hits.size(); i++)
      if (hits[i]) {
         delete hits[i];
         hits[i] = NULL;
      }
}

void FeamEvent::Print(int level) const
{
   printf("PwbEvent %d, time %f, incr %f, complete %d, error %d", counter, time, timeIncr, complete, error);
   printf(", hits: %d", (int)hits.size());
   if (modules.size() > 0) {
      printf(", modules: ");
      for (unsigned i=0; i<modules.size(); i++) {
         if (modules[i] == NULL) {
            printf(" null");
         } else {
            printf(" %d", modules[i]->cnt);
         }
      }
   }
}

void PrintFeamChannels(const std::vector<FeamChannel*>& v)
{
   printf("FeamChannels: count %d\n", (int)v.size());
   printf("### MM C R S RI CH T C RR BIN ADC\n");
   for (unsigned i=0; i<v.size(); i++) {
      if (v[i]) {
         printf("%3d %2d %d %d %d %2d %2d %d %d %2d %3d %3d\n",
                i,
                v[i]->imodule,
                v[i]->pwb_column,
                v[i]->pwb_ring,
                v[i]->sca,
                v[i]->sca_readout,
                v[i]->sca_chan,
                v[i]->threshold_bit,
                v[i]->pad_col,
                v[i]->pad_row,
                v[i]->first_bin,
                (int)v[i]->adc_samples.size());
      }
   }
}

void Unpack(FeamAdcData* a, FeamModuleData* m)
{
   int f = m->fDataFormat;
   assert(f==1 || f==2);

   a->nsca  = 4; // 0,1,2,3
   a->nchan = 79; // 1..79 readout channels per table 2 in AFTER SCA manual
   a->nbins = 511; // 0..511

   memset(a->adc, 0, sizeof(a->adc));

   const unsigned char* ptr = (const unsigned char*)m->fPtr;
   int count = 0;

   for (int ibin = 0; ibin < 511; ibin++) {
      for (int ichan = 0; ichan <= 3; ichan++) {
         for (int isca = 0; isca < 4; isca++) {
            a->adc[isca][ichan][ibin] = 0;
         }
      }
   }

   for (int ibin = 0; ibin < 511; ibin++) {
      for (int ichan = 4; ichan <= 79; ichan++) {
         for (int isca = 0; isca < 4; isca++) {
            unsigned v = ptr[0] | ((ptr[1])<<8);
            // manual sign extension
            if (v & 0x8000)
               v |= 0xffff0000;
            //if (isca == 0) {
            //   adc[ichan][ibin] = v;
            //}
            if (f==1)
               a->adc[isca][ichan][ibin] = ((int)v)/16;
            else if (f==2)
               a->adc[isca][ichan][ibin] = v;
            else
               a->adc[isca][ichan][ibin] = 0xdead;
            ptr += 2;
            count += 2;
         }
      }
   }

   //printf("count %d\n", count);
}

PwbModuleMap::PwbModuleMap() // ctor
{
   // empty
}

PwbModuleMap::~PwbModuleMap() // ctor
{
   for (unsigned i=0; i<fMap.size(); i++) {
      if (fMap[i]) {
         delete fMap[i];
         fMap[i] = NULL;
      }
   }
}

void PwbModuleMap::Print() const
{
   printf("PwbModuleMap: %d modules, %d map entries:\n", fNumModules, (int)fMap.size());
   for (unsigned i=0; i<fMap.size(); i++) {
      if (fMap[i]) {
         printf("map[%2d] - pwb%02d, column %2d, row %2d\n", i, fMap[i]->fModule, fMap[i]->fColumn, fMap[i]->fRing);
      }
   }
}

void PwbModuleMap::LoadFeamBanks(const std::vector<std::string> banks)
{
   if (banks.size() <= 8) { // short TPC
      int iring = 0;
      for (unsigned icolumn = 0; icolumn < banks.size(); icolumn++) {
         unsigned imodule = 0;

         if (banks[icolumn][0] == 'p' && banks[icolumn][1] == 'w' && banks[icolumn][2] == 'b') {
            int c2 = banks[icolumn][3] - '0';
            int c3 = banks[icolumn][4] - '0';
            imodule = c2*10 + c3;
         } else {
            int c2 = banks[icolumn][2] - '0';
            int c3 = banks[icolumn][3] - '0';
            imodule = c2*10 + c3;
         }

         if (imodule > PWB_MODULE_LAST) {
            printf("PwbModuleMap::LoadFeamBanks: Invalid module number %d in bank name [%s]\n", imodule, banks[icolumn].c_str());
            continue;
         }
 
         PwbModuleMapEntry *e = new PwbModuleMapEntry;
         e->fModule = imodule;
         e->fColumn = icolumn;
         e->fRing   = iring;

         while (imodule >= fMap.size()) {
            fMap.push_back(NULL);
         }

         fMap[imodule] = e;
         fNumModules++;
      }
   } else { // long TPC
      for (unsigned i = 0; i < banks.size(); i++) {
         unsigned imodule = 0;

         if (banks[i][0] == 'p' && banks[i][1] == 'w' && banks[i][2] == 'b') {
            int c2 = banks[i][3] - '0';
            int c3 = banks[i][4] - '0';
            imodule = c2*10 + c3;
         } else {
            int c2 = banks[i][2] - '0';
            int c3 = banks[i][3] - '0';
            imodule = c2*10 + c3;
         }

         if (imodule > PWB_MODULE_LAST) {
            printf("PwbModuleMap::LoadFeamBanks: Invalid module number %d in bank name [%s]\n", imodule, banks[i].c_str());
            continue;
         }

         int icolumn = i/8;
         int iring = i%8;

         PwbModuleMapEntry *e = new PwbModuleMapEntry;
         e->fModule = imodule;
         e->fColumn = icolumn;
         e->fRing   = iring;

         while (imodule >= fMap.size()) {
            fMap.push_back(NULL);
         }

         fMap[imodule] = e;
         fNumModules++;
      }
   }
}

const PwbModuleMapEntry* PwbModuleMap::FindPwb(int imodule)
{
   assert(imodule >= 0);
   if (imodule >= fMap.size() || !fMap[imodule]) {
      static PwbModuleMapEntry* unmapped_pwb = NULL;
      if (!unmapped_pwb) {
         unmapped_pwb = new PwbModuleMapEntry();
         unmapped_pwb->fModule = -1;
         unmapped_pwb->fColumn = -1;
         unmapped_pwb->fRing = -1;
      }
      return unmapped_pwb;
   }

   assert(fMap[imodule]->fModule == imodule);
   return fMap[imodule];
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
