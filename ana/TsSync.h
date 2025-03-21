//
// TsSync.h
// Timestamp synchronization
// K.Olchanski
//

#ifndef TsSyncH
#define TsSyncH

#include <stdio.h>
#include <stdint.h>
#include <vector>

struct TsSyncEntry
{
   uint32_t ts;
   int epoch;
   double time;

   TsSyncEntry(uint32_t xts, int xepoch, double xtime); // ctor
};

class TsSyncModule
{
 public: // configuration
   double   fFreqHz; // timestamp clock frequency, Hz
   double   fEpsSec; // time comparison threshold, sec
   double   fRelEps; // relative time comparison threshold, sec/sec
   unsigned fBufMax; // buffer overflow limit, nevents
   
 public: // running data
   int      fEpoch;
   uint32_t fFirstTs;
   uint32_t fPrevTs;
   uint32_t fLastTs;
   double   fOffsetSec;
   double   fPrevTimeSec;
   double   fLastTimeSec;
   double   fMaxDtSec;
   double   fMaxRelDt;

 public: // final status
   int      fSyncedWith; // =(-1) not synchronized, >= 0 synchronized with module
   bool     fOverflow;   // buffer overflow
   bool     fDead;       // module has no events, declared dead

 public: // timestamp buffer
   std::vector<TsSyncEntry> fBuf;

public:
   TsSyncModule(); // ctor
   void Print() const;
   void DumpBuf() const;
   double GetTime(uint32_t ts, int epoch) const;
   void Add(uint32_t ts);
   void Retime();
   double GetDt(unsigned j);
   unsigned FindDt(double dt);
};

class TsSync
{
public:
   std::vector<TsSyncModule> fModules;
   bool fSyncOk;
   bool fSyncFailed;
   bool fOverflow;
   bool fTrace;
   unsigned fDeadMin;

public:
   TsSync(); // ctor
   ~TsSync(); // dtor
   void SetDeadMin(int dead_min);
   void Configure(unsigned i, double freq_hz, double eps_sec, double rel_eps, int buf_max);
   void CheckSync(unsigned ii, unsigned i);
   void Check(unsigned inew);
   void Add(unsigned i, uint32_t ts);
   void Dump() const;
   void Print() const;
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
