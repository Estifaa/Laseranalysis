

void intensity() {
   //Draw a graph with error bars
   // To see the output of this macro, click begin_html <a href="gif/gerrors.gif">here</a>. end_html
   //Author: Rene Brun

   TCanvas *c1 = new TCanvas("c1","A Simple Graph with error bars",200,10,700,500);

   c1->SetFillColor(42);
   c1->SetGrid();
   c1->GetFrame()->SetFillColor(21);
   c1->GetFrame()->SetBorderSize(12);

   const Int_t n = 22;
   Float_t x1[n]  = {20, 30, 42, 54.7, 67.4, 92.8, 118.2, 143.6, 169, 194.4, 219.8, 245.2, 270.6, 296, 550, 804, 1058, 1312, 1566, 1820, 2074, 2328};
   Float_t y1[n]  = {1.6, 4.68, 95.4, 164.4, 182.4, 194, 180, 176.4, 140, 122, 90.7, 79.4000794000794, 66.7003335016675, 56.2017422540099, 15.7960544738221, 9.62713709294761, 7.26357653745703, 5.08932376376953, 4.87148597908954, 3.12996533114834, 1.68023666312149, 1.0510919953395};
   Float_t ex[n] = {.05,.1,.07,.07,.04,.05,.06,.07,.08,.05};
   Float_t ey[n] = {.8,.7,.6,.5,.4,.4,.5,.6,.7,.8};
   TGraphErrors *gr1 = new TGraphErrors(n,x1,y1,ex,ey);
   gr1->SetTitle("TGraphErrors Example");
   gr1->SetMarkerColor(4);
   gr1->SetMarkerStyle(21);
   gr1->Draw("ALP");

   c1->Update();
}

 
