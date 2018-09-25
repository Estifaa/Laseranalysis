
#include "TCanvas.h"
#include "TMath.h"
#include "TH1.h"
#include "TF1.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "unistd.h"

Int_t npeaks = 9;
Double_t fpeaks(Double_t *x, Double_t *par) {
  Double_t result = par[0] + par[1]*x[0];
  for (Int_t p=0;p<npeaks;p++) {
    Double_t norm  = par[3*p+2];
    Double_t mean  = par[3*p+3];
    Double_t sigma = par[3*p+4];
    result += norm*TMath::Gaus(x[0],mean,sigma);
  }
  return result;
}
void Tspectrum()
{



  TString index_s;
  TString filename1 ="~/lmartin/agdaq_laser/ana/output02302.root"; 
  TFile *f = new TFile(filename1);
  cout <<f <<endl;
  TCanvas *c1 = new TCanvas("c1","Profile histogram example",1200,1000); 
  c1 ->Draw();
  TH2D *hfix = (TH2D*) f->FindObjectAny("h_pad_amp_per_row_cut"); 
  Int_t NX = hfix-> GetNbinsX();
  cout << "number of bins " << NX <<endl;
 

  //hfix->GetXaxis()->SetRangeUser(0,400);
  for (Int_t r=0; r < NX ;r++){
    index_s.Form("%i",r);
    TH1D *py = hfix->ProjectionY("py"+index_s,r,r,"d"); 
    Double_t StdDev = py ->GetStdDev();
  
    //cout<< StdDev;
    py ->SetFillColor(kBlue-2);
    if(StdDev < 500 && StdDev !=0 && py ->GetEntries()> 500 ){
      cout<<index_s<<endl; 
      //TProfile *hprof=hfix -> ProfileX ();
      //Use TSpectrum to find the peak candidates
      TSpectrum *s = new TSpectrum(4);
      Int_t nfound = s->Search(py,20,"",0.05);
      cout << "Found "<< nfound << " candidate peaks to fit." << endl;
      Double_t *xpeaks = s->GetPositionX();
      Double_t *ypeaks = s->GetPositionY();
      cout << "Now fitting: Be patient.\n";
      TF1 *f1 = new TF1("f1",fpeaks,0,1000,2+3*nfound);
      Double_t par[2000];
      par[0] = 0.1;
      par[1] = 1.;
      for (int p=0;p<nfound;p++) {
	Double_t xp = xpeaks[p];
	Double_t yp = ypeaks[p];
	par[3*p+2] = yp;
	par[3*p+3] = xp;
	par[3*p+4] = 50;
      }
      

      // TVirtualFitter::Fitter(hfix,10+3*nfound);
      f1->SetParameters(par);
      f1->FixParameter(1,0);
      //cout << "parameters " << f1->GetParameter(0) << ", " << f1->GetParameter(1) << ", " << f1->GetParameter(2) << ", " << f1->GetParameter(3) << ", " << f1->GetParameter(4) << endl;
      f1->SetNpx(1000);
      py ->Draw(); 
      py->Fit("f1");
      cout << "Now fitting: Be patient.\n";

      // cout<<index_s<<endl; 
      //cout<<StdDev<<endl;
      c1 ->Update();
      sleep(1);

    }
    //cout<<index_s<<endl; 
  }
  
  return;
 
  






































  TProfile *hprof=hfix -> ProfileX ();
  //hprof->SetLineStyle(1);
  //hprof->SetLineColor(kBlack);
  //hprof -> Draw();

  //Use TSpectrum to find the peak candidates
  TSpectrum *s = new TSpectrum(2*npeaks);
  Int_t nfound = s->Search(hfix,0.5,"",0.10);
  cout << "Found candidate peaks to fit.\n";
  cout << nfound;
  Double_t *xpeaks = s->GetPositionX();
  Double_t *ypeaks = s->GetPositionY();
  //Estimate background using TSpectrum::Background
  TH1 *hb = s->Background(hfix,20,"same");
  if (hb) c1->Update();

  return;
  
  cout << "Now fitting: Be patient.\n";
  TF1 *f1 = new TF1("f1",fpeaks,0,1000,2+3*npeaks);
  Double_t par[2000];
  for (int p=0;p<nfound;p++) {
    Double_t xp = xpeaks[p];
    Double_t yp = ypeaks[p];
    par[3*npeaks+2] = yp;
    par[3*npeaks+3] = xp;
    par[3*npeaks+4] = 3;
    npeaks++;
  }
  TVirtualFitter::Fitter(hfix,10+3*npeaks);
  f1->SetParameters(par);
  f1->SetNpx(1000);
  hfix->Fit(f1);
  //hfix->Fit(f1,"w");
  //hfix->Fit(f1,"ww");
  cout << "Now fitting: Be patient.\n";

  

}


