
#include "TCanvas.h"
#include "TMath.h"
#include "TGraph.h"
#include "TH1.h"
#include "TF1.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "unistd.h"
#include "TNamed.h"
#include "TAttLine.h"
#include "TAttFill.h"
#include "TAttMarker.h"
#include "TVectorFfwd.h"
#include "TVectorDfwd.h"


Int_t npeaks = 3;
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
void Tspectrum_T11()
{


  Double_t StdDev;
  Double_t Mean;
  TString index_s;
  TString index_r;
  TString filename1 ="~/lmartin/agdaq_laser/ana/output02303.root"; 
  TString object = "h_pad_amp_per_row_cut_col";
  TFile *f = new TFile(filename1);
 
 
  
  TCanvas *c2 = new TCanvas("c2","A Graph ",200,10,700,500);
  TH1D* hdummy= new TH1D ("hdummy", "peaks graph T11", 1, 0., 576.);
  hdummy ->SetStats(kFALSE); //gets rid of the box 
  hdummy ->Draw();
  hdummy ->GetYaxis() ->SetRangeUser(0.,2000.);
  double maxpeak=0.;
  c2->SetFillColor(kGray);
  c2->SetGrid();
  auto legend = new TLegend(0.1,0.7,0.48,0.9);

  for(Int_t i=15; i<22; i++){
    index_r.Form("%i",i);
    // TString filename = object+index_r;
    //filename = TString::Format("%s%02i",object.Data(),i);
    // filename = TString::Format("h_pad_amp_per_row_cut_col%02i",i);
    TString filename = object+ TString::Format("%02i",i);                      

    TCanvas *c1 = new TCanvas("c1"+index_r,"Profile histogram example",1200,1000); 
    //c1 ->Draw();
    c1 -> Iconify();
    TH2D *hfix = (TH2D*) f->FindObjectAny(filename); 
    cout<<filename<<endl;
    Int_t NX = hfix-> GetNbinsX();
    cout << "number of bins " << NX <<endl;
   

    vector<double> x, y;

    //hfix->GetXaxis()->SetRangeUser(0,400);
    if ( i <21){
      for (Int_t r=0; r < NX ;r++){
	index_s.Form("%i",r);
	TH1D *py = hfix->ProjectionY("py"+index_s+"col"+index_r,r,r,"d"); 
	py ->SetFillColor(kBlue-2);
	StdDev = py ->GetStdDev();
	Mean = py ->GetMean();

	if(StdDev < 500 && StdDev !=0 && py ->GetEntries()> 50 ){

	  //Use TSpectrum to find the peak candidates
	  x.push_back(r);
	  y.push_back(Mean); 
	  //printf( " %f %f \n" , x.back(), y.back()); 
	
	  TSpectrum *s = new TSpectrum(2);
	  Int_t nfound = s->Search(py,20,"",0.1);
	  cout << "Found "<< nfound << " candidate peaks to fit." << endl;
	  Double_t *xpeaks = s->GetPositionX();

	  Double_t *ypeaks = s->GetPositionY();
	  //cout << "xpeaks " << xpeaks <<endl;
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
	  // sleep(1);

	} else if(py ->GetEntries()>1000 && StdDev != 0){
	  cout << r << " : StdDev = " << StdDev << " py ->GetEntries() = " <<  py ->GetEntries() << endl;
	  py ->Draw();
	  c1 ->Update();
	  //sleep(10);	
	}
	
      }
      
    
    }
    else{
      for (Int_t r=300; r < NX ;r++){
	index_s.Form("%i",r);
	TH1D *py = hfix->ProjectionY("py"+index_s+"col"+index_r,r,r,"d"); 
	py ->SetFillColor(kBlue-2);
	StdDev = py ->GetStdDev();
	Mean = py ->GetMean();

	if(StdDev < 500 && StdDev !=0 && py ->GetEntries()> 50 ){
	  
	  //Use TSpectrum to find the peak candidates
	  x.push_back(r);
	  y.push_back(Mean); 
	  printf( " %f %f \n" , x.back(), y.back()); 
	 
	  TSpectrum *s = new TSpectrum(2);
	  Int_t nfound = s->Search(py,20,"",0.1);
	  cout << "Found "<< nfound << " candidate peaks to fit." << endl;
	  Double_t *xpeaks = s->GetPositionX();
	  Double_t *ypeaks = s->GetPositionY();
	  //cout << "xpeaks " << xpeaks <<endl;
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
	  //sleep(1);
	}
	else if(py ->GetEntries()>1000 && StdDev != 0){
	  cout << r << " : StdDev = " << StdDev << " py ->GetEntries() = " <<  py ->GetEntries() << endl;
	  py ->Draw();
	  c1 ->Update();
	  //sleep(10);	
	}
      }
    }
      
      
      
    
    // TH1D* pad_peak_per_row;
    // pad_peak_per_row = new TH1D();
    // pad_peak_per_row -> Fill (x[n],y[n]);

    //new TCanvas;

    // TH1* h1 = new TH1D("h1", "peaks", 100, 0.0, 4.0);
    //h1 ->Fill(x.data(),y.data());
    // TCanvas *c2 = new TCanvas("c2","A Graph ",200,10,700,500);
    //c2->SetFillColor(42);
    //c2->SetGrid();



    TGraph *gr = new TGraph(x.size(),x.data(),y.data());
    gr ->SetName("col" +index_r);
    gr->SetLineColor(i-13);
    gr->SetLineWidth(1);
    gr->SetMarkerColor(i-13);
    gr->SetMarkerStyle(21);
    gr->SetTitle("col" +index_r);
    gr->GetXaxis()->SetTitle("pad rows");
    gr->GetYaxis()->SetTitle("amp");
    //maxpeak=maxpeak>*std::max_element(y.begin(),y.end())?maxpeak:*std::max_element(y.begin(),y.end());
  
    //c2 ->cd();
    //gr->Draw("CP SAME");
    //c2 ->Update();
  
   
    TH1* h1 = new TH1D("h1 col " +index_r, "peaks", NX, -0.5, NX-0.5);
    // h1 ->Fill(y.data());
    auto nPoints = gr->GetN();
    //cout << "number of Points is: " << nPoints << endl;
    for(int i=0; i < nPoints; ++i) {
      double x,y;
      gr->GetPoint(i, x, y);
      cout << x << y <<endl;
      h1->Fill(x,y); 
    }
    // h1 ->Draw();


    TSpectrum *s = new TSpectrum(9);
    Int_t nfound = s->Search(h1,3,"",0.05);
    cout << "Found "<< nfound << " candidate peaks to fit." << endl;
    Double_t *x1peaks = s->GetPositionX();
    Double_t *y1peaks = s->GetPositionY();
    cout << "x1peaks " << x1peaks <<endl;
    cout << "Now fitting: Be patient.\n";
    TF1 *f2 = new TF1("f2",fpeaks,0,1000,2+3*nfound);
    Double_t par[2000];
    par[0] = 0.1;
    par[1] = 1.;
    for (int p=0;p<nfound;p++) {
      Double_t x1p = x1peaks[p];
      Double_t y1p = y1peaks[p];
      par[3*p+2] = y1p;
      par[3*p+3] = x1p;
      par[3*p+4] = 2;
    }


    c2 ->cd();
    //  new TCanvas; 
    f2->SetParameters(par);
    f2->FixParameter(1,0);
    f2->FixParameter(0,0);
    //cout << "parameters " << f1->GetParameter(0) << ", " << f1->GetParameter(1) << ", " << f1->GetParameter(2) << ", " << f1->GetParameter(3) << ", " << f1->GetParameter(4) << endl;
    f2->SetNpx(1000);
    //h1 ->Draw ();

    
    const Int_t n = 22;
   
    Float_t x1[n] = {5, 7.5, 10.5, 13.675, 16.85, 23.2, 29.55, 35.9, 42.25, 48.6, 54.95, 61.3, 67.65, 74, 137.5, 201, 264.5, 328, 391.5, 455, 518.5, 582};
    //{50, 75, 105, 136.75, 168.5, 232, 295.5, 359, 422.5, 486, 549.5, 613, 676.5, 740, 1375, 2010, 2645, 3280, 3915, 4550, 5185, 5820};
    Float_t y1[n]  = {256, 748.8, 15264, 26304, 29184, 31040, 28800, 28224, 22400, 19520, 14512, 12704.0127040127, 10672.0533602668, 8992.27876064158, 2527.36871581154, 1540.34193487162, 1162.17224599313, 814.291802203125, 779.437756654327, 500.794452983734, 268.837866099438, 168.17471925432};
    //{168.17471925432, 268.83786609943, 500.794452983734, 779.437756654327, 814.291802203125, 1162.17224599313, 1540.34193487162, 2527.36871581154, 8992.27876064158, 10672.0533602668, 12704.0127040127, 14512, 19520, 22400, 28224, 28800, 31040, 29184, 26304, 15264, 748.8, 256};


    // {256, 748.8, 15264, 26304, 29184, 31040, 28800, 28224, 22400, 19520, 14512, 12704.0127040127, 10672.0533602668, 8992.27876064158, 2527.36871581154, 1540.34193487162, 1162.17224599313, 814.291802203125, 779.437756654327, 500.794452983734, 268.837866099438, 168.17471925432};

  
    Float_t ex[n] = {.05,.1,.07,.07,.04,.05,.06,.07,.08,.05};
    Float_t ey[n] = {.8,.7,.6,.5,.4,.4,.5,.6,.7,.8};
    
    Float_t x2[n];
    Float_t y2[n];
    double c = -1; 
    double d = 582; 
    double b = 0.3;
    for(int i=0;i<n;i++){
      x2[i] = (c*x1[i])+ d;
      y2[i] = b*y1[i];
      //return x2;
    
    }
    
    TGraphErrors *gr1 = new TGraphErrors(n,x2,y2,ex,ey);
    gr1->SetTitle("TGraphErrors Example");
    gr1->SetMarkerColor(4);
    gr1->SetMarkerStyle(21);
    gr1->SetLineColor(1);
    gr1->SetLineWidth(4);
    gr1->Draw("SAME");

    h1 ->Draw("LP same");
    h1->Fit("f2");
    cout << "Now fitting: Be patient.\n";

    // cout<<index_s<<endl; 
    //cout<<StdDev<<endl;
    //c3 ->Update();
   
   
    //h1 ->Fit ("f1","R");
    //gr->Draw("CP SAME");
    //gr->Draw();
    //c2 ->Update();



    
    gr->Draw((i==0)?"ALP":"LP same");
    
   
 

    // cout<<index_s<<endl; 
    //cout<<StdDev<<endl;
    //c3 ->Update();
   
   
    //h1 ->Fit ("f1","R");
    //gr->Draw("CP SAME");
    //gr->Draw();
    //c2 ->Update();



    
    gr->Draw((i==0)?"ALP":"LP same");
  
    legend->SetHeader("Peaks","C"); // option "C" allows to center the header
    legend->AddEntry(h1,"h1 col" +index_r,"f");
    legend->AddEntry("gr","gr col"+index_r,"lep");
    //c2->Update();
  
  }
  legend->Draw();
  // hdummy ->GetYaxis() ->SetRangeUser(0.,maxpeak*1.1);
 
}
