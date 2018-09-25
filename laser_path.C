#include "TMath.h"

Double_t laser_profile(Double_t *x, Double_t *par)
{
  double a = TMath::Cos(par[4])*TMath::Cos(par[3]),
    b = TMath::Sin(par[4]),
    c = TMath::Cos(par[4])*TMath::Sin(par[3])/par[1],
    d = (-TMath::Cos(par[4])*TMath::Sin(par[3])*par[0]+par[2]*TMath::Sin(par[4]))/par[1];
  double gamma = TMath::Sqrt(a*a+b*b),
    delta = TMath::ATan2(b,a);
  double phi =(((TMath::ASin((c*(-x[0]+576)+d/gamma)-delta) + par[5]))*TMath::RadToDeg())/11.25;
  //double phi =(((TMath::ASin((c*x[0]+d/gamma)-delta) + par[5]))*TMath::RadToDeg())/11.25; 
  // double phimax = TMath::Cos(par[1]/par[2]) + par[5];
  // if( phi > phimax ) return 0.;
  return phi;
}

void laser_path()
{


  TString index_s;
  TString filename,outputfilename;
  //TString folder = "/home/alphag/agdaq/ana/output0";
  TString folder = "~/lmartin/agdaq_laser/ana/output0";

  cout<<"Name of output file (without .root): ";
  cin>> outputfilename;
  cout<<endl;

  TFile* histograms = new TFile(outputfilename+".root","RECREATE","histograms");
  // histograms->Close();

  int nh=0;
  for(Int_t i=2301; i<2317; i++){
    if(i!=2311){
      index_s.Form("%i",i);
      filename= folder+index_s+".root";
      cout<<filename<<endl;

      //filename = "/home/alphag/agdaq/ana/output02300.root";
      TFile *f = new TFile(filename,"READ"); 

      f->cd("final/summary");

      TString cname = TString::Format("c%d",i);
      TCanvas *c1  = new TCanvas(cname.Data(),cname.Data(),1200,1000);

     
      TH2D *hfix = (TH2D*) gROOT->FindObject("h_pad_hits_per_row_column");
      //TH2D *hfix = (TH2D*) gROOT->FindObject("h_pad_amp_time");

      cout<<hfix->GetEntries()<<endl;
      /*
	double z0 = -1172.5; //mm 
	double beam_inclination1 = 3.0; //deg
	double plane_inclination1 = -50.0; //deg
	double cathode_radius = 109.25; // mm
	double rod_pos = 27.35; // mm
	double Rr = cathode_radius + rod_pos;
	double off_phi_deg = 25.; //deg
	double off_phi = (off_phi_deg * TMath::Pi())/180; 

	double target_phi2(z, z0, Rc, Rr, theta, beta){
	double a = cos(beta)*cos(theta);
	double b = sin(beta);
	double c = cos(beta)*sin(theta)/Rc;
	double d = (-cos(beta)*sin(theta)*z0 + Rr*sin(beta))/Rc;
	double gamma = sqrt(a*a + b*b);
	double delta = atan2(b,a);
	return asin((c*z+d)/gamma) - delta;
	}
      */


      // z0 -> par[0]
      // Rc -> par[1]
      // Rr -> par[2]
      // theta -> par[3]
      // beta -> par[4]
      // off_phi -> par[5]


      //c1->Divide(1,2); 

      //c1->cd();
      //hfix->SetMarkerStyle(8);
      hfix ->Draw ();
      //hfix->GetXaxis() ->SetRangeUser(-1152.,1152.);
      //c1 ->Update();
      
      double z0 = -5.125, Rc = 27.3, Rr = 34.12, // mm
	theta = -3.0*TMath::DegToRad(), beta = -50.*TMath::DegToRad();
      double off_phi = (5*TMath::DegToRad())/11.25;
      double zmin=0., zmax=576.; // rows
      TF1* f1 = new TF1("laser_profile_1",laser_profile,zmin,zmax,6);
      f1->SetParameters(z0,Rc,Rr,theta,beta,off_phi);
      f1->SetParNames("z0","Rc","Rr","theta","beta");
      f1->Draw("SAME");


      //TFile* ofile = new TFile(outputfilename+".root","UPDATE","histograms");
      histograms->cd();
      c1->Write();

      //ofile->Close();
    }
    ++nh;
    cout<<"+++++++++ " <<nh << " +++++++++++++++"<<endl;
  }
}
