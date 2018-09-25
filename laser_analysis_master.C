/* solution 1, but the codes are different
#include "Tspectrum_B07.C"
#include "Tspectrum_B15.C"
#include "Tspectrum_T03.C"
#include "Tspectrum_T11.C"

void laser_analysis_master()
{
  Tspectrum_B07();
  Tspectrum_B015();
  Tspectrum_T03();
  Tspectrum_B11();
}
*/

/*solution 2 THE BEST
#include "Tspectrum_merged.C" // this file is the result of the clean-up
// and it will contain the following function
void Tspectrum_merged(int* regions, int Nregions)
{
  // ...

  for(int n=0; n<Nregions; ++n)
    {
      Int_t i= regions[n];
      index_r.Form("%i",i);
    }


}
*/

void laser_analysis_master()
{
  int wanted_regions_B15[]={0,3,4,5,23,24,25,26,27,28,29,30,31};
  Tspectrum_merged(wanted_regions_B15,13);

  int wanted_regions_B07[]={4,5,6};
  Tspectrum_merged(wanted_regions_B07,3);
}
