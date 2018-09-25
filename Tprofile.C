
void Tprofile()
{
  TString index_s;
  TString filename,outputfilename;
TString folder = "~/Estifaa/rootfiles/output0";

 TCanvas *c1;

 cout<<"Name of output file (without .root): ";
  cin>> outputfilename;
  cout<<endl;

  TFile* histograms = new TFile(outputfilename+".root","RECREATE","histograms");
  histograms->Close();

  int nh=0;
  for(Int_t i=2300; i<2303; i++){
    if(i!=2311){
      index_s.Form("%i",i);
      filename= folder+index_s+".root";
      cout<<filename<<endl;

 TFile *f = new TFile(filename); 

      f->cd("final/summary");


      //  TH2D *hfix = (TH2D*) gROOT->FindObject("h_aw_amp_time");
      
      //  cout<<hfix->GetEntries()<<endl;
 
       // TH2D *hfix1 = (TH2D*) gROOT->FindObject("h_aw_map_time");

      //  cout<<hfix1->GetEntries()<<endl;  
      
      //  TProfile *hprof=hfix -> ProfileX ();


      //   TProfile *hprof1=hfix1 -> ProfileX ();


         TH2D *hfix2 = (TH2D*) gROOT->FindObject("h_aw_map_amp");

        TString pname = TString::Format("%sR%d_px",hfix2 ->GetName(),i );
      TProfile *hprof2=hfix2 -> ProfileX (pname.Data());

 if( nh==0){
	c1 = new TCanvas("c"+index_s,"Profile histogram example",1200,1000);
	hprof2 -> Draw();

      }
      else{
	hprof2 -> Draw("SAME");

      }
      hprof2 -> SetTitle("Signal Amplitude vs Anode wire position");
       hprof2-> SetXTitle("Anode wire");
      
       hprof2->SetYTitle("Amplitude");
      hprof2->GetXaxis()->SetRange(200,500);
   TFile* ofile = new TFile(outputfilename+".root","UPDATE","histograms");

      c1->Write();

      ofile->Close();
    }
    ++nh;
   
}

