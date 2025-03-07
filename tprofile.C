

void tprofile()
{
TString index_s;
TString filename,outputfilename;
  //TString folder = "/home/alphag/agdaq/ana/output0";
TString folder = "~/Estifaa/rootfiles/output0";



cout<<"Name of output file (without .root): ";
cin>> outputfilename;
cout<<endl;

TFile* histograms = new TFile(outputfilename+".root","RECREATE","histograms");
histograms->Close();

for(Int_t i=2300; i<2303; i++){
if(i!=2311){
index_s.Form("%i",i);
filename= folder+index_s+".root";
cout<<filename<<endl;

//filename = "/home/alphag/agdaq/ana/output02300.root";
TFile *f = new TFile(filename); 

f->cd("final/summary");


TH2D *hfix = (TH2D*) gROOT->FindObject("h_aw_amp_time");
//TH2D *hfix = (TH2D*) gROOT->FindObject("h_pad_amp_time");

cout<<hfix->GetEntries()<<endl;


TCanvas *c1 = new TCanvas("c"+index_s,"Profile histogram example",1200,1000);
//c1->Divide(1,2);

//c1->cd(1);
//hfix->SetMarkerStyle(8);
//hfix ->Draw ();


TH2D *hfix1 = (TH2D*) gROOT->FindObject("h_aw_map_time");
//TH2D *hfix1 = (TH2D*) gROOT->FindObject("h_pad_amp_time");
cout<<hfix1->GetEntries()<<endl;

//c1->cd(3);
//hfix1->SetMarkerStyle(8);
//hfix1 ->Draw ();


TProfile *hprof=hfix -> ProfileX ();
//TProfile *hprof10=hfix -> ProfileX ("_pfx10",10);
//hprof10->SetMarkerColor(kRed);
//hprof10->SetLineColor(kRed);
//TProfile *hprof20=hfix -> ProfileX ("_pfx20",20);
//hprof20->SetMarkerColor(kBlack);
//hprof20->SetLineColor(kBlack);
//c1->cd(2);
//cout<<"here"<<endl;
//hprof -> Draw();
//hprof -> SetTitle("Signal Amplitude vs Time");
//hprof-> SetXTitle("Time");
//hprof->SetYTitle("Signal Amplitude");
//hprof->GetXaxis()->SetRange(4000,10000);
//hprof10->Draw("same");
//hprof20->Draw("same");

TProfile *hprof1=hfix1 -> ProfileX ();
//c1->cd(1);
//cout<<"here"<<endl;
//hprof1 -> Draw();
//hprof1 -> SetTitle("Time vs Anode wire position");
//hprof1-> SetXTitle("Anode wire");
//hprof1->SetYTitle("Time");
//hprof1->GetXaxis()->SetRange(114,130);

TH2D *hfix2 = (TH2D*) gROOT->FindObject("h_aw_map_amp");
//TH2D *hfix2 = (TH2D*) gROOT->FindObject("h_pad_amp_time");
cout<<hfix2->GetEntries()<<endl;

//c1->cd(5);
//hfix2->SetMarkerStyle(8);
//hfix2->Draw ();

TProfile *hprof2=hfix2 -> ProfileX ();
//c1->cd(2);
//cout<<"here"<<endl;
hprof2 -> Draw("SAME PLC PMC");
hprof2 -> SetTitle("Signal Amplitude vs Anode wire position");
//hprof2 -> SetTitle("Signal Amplitude vs pad position");
hprof2-> SetXTitle("Anode wire");
//hprof2-> SetXTitle("pad");
hprof2->SetYTitle("Amplitude");
hprof2->GetXaxis()->SetRange(200,500);

TFile* ofile = new TFile(outputfilename+".root","UPDATE","histograms");

c1->Write();

ofile->Close();
}}
}


