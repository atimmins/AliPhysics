#if !defined(__CINT__) || defined(__MAKECINT__)
#include <TDatabasePDG.h>
#include <TRandom3.h>
#include <TMath.h>
#include <TLorentzVector.h>
#include <TParticle.h>
#include <TParticle.h>
#include <TClonesArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TPythia6Decayer.h>
#include <TPaveStats.h>
#include "AliVertexingHFUtils.h"
#endif

enum EDDecay{kD0Kpi,kDplusKpipi,kDstarD0pi,kDsKKpi,kLcpKpi,kLcK0Sp,kLcpiL,kDplusKKpi};
enum EFidY{kFixedY,kPtDepY};
enum EPtShape{kFlatPt,kPtFONLL8TeV,kPtFONLL8TeVfeeddown,kPtFONLL7TeV,kPtPythia7TeV,kPtFONLL5TeV,kPtFONLL13TeVprompt,kPtFONLL13TeVfeeddown,kPtPythia13TeVprompt,kPtPythia13TeVfeeddown};
enum EYShape{kFlatY,kYFONLL5TeV,kYFONLLMax5TeV,kYFONLLMin5TeV,kYFONLL13TeV,kYFONLLMax13TeV,kYFONLLMin13TeV};

// Configuration
Int_t fDDecay=kD0Kpi;
Double_t fPtMinDau=0.1;
Double_t fPtMinDauSoftPi=0.06; // only for D*
Double_t fEtaMaxDau=0.9;
Int_t fOptionYFiducial=kPtDepY;
Double_t fYMaxFidAccCut=0.8;
Int_t fPtShape=kPtFONLL5TeV;
Int_t fYShape=kYFONLL5TeV;
TString fDecayTableFileName="$ALICE_PHYSICS/PWGHF/vertexingHF/macros/decaytable_acc.dat"; 
Int_t fDebugLevel=0;
Int_t totTrials=10000000;


Bool_t CountKpi(TClonesArray *array, Int_t nentries, Int_t &nPions, Int_t &nKaons, Int_t &nPionsInAcc, Int_t &nKaonsInAcc);
Bool_t IsInFiducialAcceptance(Double_t pt, Double_t y);
Bool_t CountPKpi(TClonesArray *array, Int_t nentries, Int_t &nPions, Int_t &nKaons, Int_t &nProtons, Int_t &nPionsInAcc, Int_t &nKaonsInAcc, Int_t &nProtonsInAcc, Int_t &idLcResChan, std::vector<Int_t> &pdgDauAll);


// Pt-shape histograms
TH1D* LoadFONLL13TeV_promptD0();
TH1D* LoadFONLL13TeV_promptDplus();
TH1D* LoadFONLL13TeV_promptDstar();
TH1D* LoadFONLL13TeV_feeddownD();
TH1D* LoadFONLL13TeV_feeddownDstar();
TH1D* LoadPYTHIA13TeV_promptD0();
TH1D* LoadPYTHIA13TeV_promptDplus();
TH1D* LoadPYTHIA13TeV_promptDstar();
TH1D* LoadPYTHIA13TeV_promptDs();
TH1D* LoadPYTHIA13TeV_promptLc();
TH1D* LoadPYTHIA13TeV_feeddownD0();
TH1D* LoadPYTHIA13TeV_feeddownDplus();
TH1D* LoadPYTHIA13TeV_feeddownDstar();
TH1D* LoadPYTHIA13TeV_feeddownDs();
TH1D* LoadPYTHIA13TeV_feeddownLc();



void ComputeAcceptance(){
  // main function
  
  gSystem->Load("liblhapdf.so");      // Parton density functions
  gSystem->Load("libEGPythia6.so");   // TGenerator interface
  gSystem->Load("libpythia6.so");     // Pythia
  gSystem->Load("libAliPythia6");  // ALICE specific implementations

  TPythia6Decayer* pdec=TPythia6Decayer::Instance();
  if(fDecayTableFileName.CompareTo("")!=0){
    if(fDecayTableFileName.Contains("ALICE_PHYSICS")){
      gSystem->Exec(Form("cp %s .",fDecayTableFileName.Data()));
      fDecayTableFileName.ReplaceAll("$ALICE_PHYSICS/PWGHF/vertexingHF/macros/","./");
    }
    pdec->SetDecayTableFile(fDecayTableFileName.Data());
    pdec->ReadDecayTable();
  }
  pdec->Init();

  Int_t pdgCode=0;
  Int_t nPionDau=-1;
  Int_t nProtonDau=-1;
  Int_t nKaonDau=-1;
  TString outFileName="Acceptance_Toy_";
  if(fDDecay==kD0Kpi){
    pdgCode=421;
    nPionDau=1;
    nKaonDau=1;
    nProtonDau=0;
    outFileName.Append("D0Kpi_");
  }else if(fDDecay==kDplusKpipi){
    pdgCode=411;
    nPionDau=2;
    nKaonDau=1;
    nProtonDau=0;
    outFileName.Append("DplusKpipi_");
  }else if(fDDecay==kDstarD0pi){
    pdgCode=413;
    nPionDau=2;
    nKaonDau=1;
    nProtonDau=0;
    outFileName.Append("DStarD0pi_");
  }else if(fDDecay==kDsKKpi){
    pdgCode=431;
    nPionDau=1;
    nKaonDau=2;
    nProtonDau=0;
    outFileName.Append("DsKKpi_");
  }else if(fDDecay==kLcpKpi){
    pdgCode=4122;
    nPionDau=1;
    nKaonDau=1;
    nProtonDau=1;
    outFileName.Append("LcpKpi_");
  }else if(fDDecay==kLcK0Sp){
    pdgCode=4122;
    nPionDau=2;
    nKaonDau=0;
    nProtonDau=1;
    outFileName.Append("LcK0Sp_");
  }else if(fDDecay==kLcpiL){
    pdgCode=4122;
    nPionDau=2;
    nKaonDau=0;
    nProtonDau=1;
    outFileName.Append("LcpiL_");
  }else if(fDDecay==kDplusKKpi){
    pdgCode=411;
    nPionDau=1;
    nKaonDau=2;
    nProtonDau=0;
    outFileName.Append("DplusKKpi_");
  }else{
    printf("ERROR: Wrong decay selected\n");
    return;
  }
  if(fOptionYFiducial==kFixedY) outFileName.Append(Form("yfid%02d_",(Int_t)(fYMaxFidAccCut*10)));
  else outFileName.Append("yfidPtDep_");
  outFileName.Append(Form("etaDau%02d_",(Int_t)(fEtaMaxDau*10)));
  outFileName.Append(Form("ptDau%d_",(Int_t)(fPtMinDau*1000)));
  TDatabasePDG* db=TDatabasePDG::Instance();
  Float_t massD=db->GetParticle(pdgCode)->Mass();
  TClonesArray *array = new TClonesArray("TParticle",100);

  TH2D* hPtVsYGen=new TH2D("hPtVsYGen","",500,0.,50.,20.,-1.,1.);
  hPtVsYGen->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  hPtVsYGen->GetYaxis()->SetTitle("y");
  TH2D* hPtVsYGenLimAcc=new TH2D("hPtVsYGenLimAcc","",500,0.,50.,20.,-1.,1.);
  hPtVsYGenLimAcc->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  hPtVsYGenLimAcc->GetYaxis()->SetTitle("y");
  TH2D* hPtVsYGenAcc=new TH2D("hPtVsYGenAcc","",500,0.,50.,20.,-1.,1.);
  hPtVsYGenAcc->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  hPtVsYGenAcc->GetYaxis()->SetTitle("y");

  // Histograms for Lc->pKpi split by resonance
  TH2D* hPtVsYGenLimAccLcpKpi[4];
  TH2D* hPtVsYGenAccLcpKpi[4];
  TString lcChan[4]={"NonRes","L1520","Kstar","Delta"};
  for(Int_t ich=0; ich<4; ich++){
    hPtVsYGenLimAccLcpKpi[ich]=new TH2D(Form("hPtVsYGenLimAcc%s",lcChan[ich].Data()),"",500,0.,50.,20.,-1.,1.);
    hPtVsYGenLimAccLcpKpi[ich]->GetXaxis()->SetTitle("p_{T} (GeV/c)");
    hPtVsYGenLimAccLcpKpi[ich]->GetYaxis()->SetTitle("y");
    hPtVsYGenAccLcpKpi[ich]=new TH2D(Form("hPtVsYGenAcc%s",lcChan[ich].Data()),"",500,0.,50.,20.,-1.,1.);
    hPtVsYGenAccLcpKpi[ich]->GetXaxis()->SetTitle("p_{T} (GeV/c)");
    hPtVsYGenAccLcpKpi[ich]->GetYaxis()->SetTitle("y");
  }
  

  TF1*  funcPt=0x0;
  TH1D* histPt=0x0;
  if(fPtShape==kPtFONLL8TeV){
    funcPt=new TF1("fFONLL","[0]*x/TMath::Power((1+TMath::Power(x/[1],[3])),[2])",0.,50.);
    funcPt->SetParameters(0.518046,3.01138,3.38914,1.75899); // Prompt
    outFileName.Append("FONLL8ptshape_");
  }else if(fPtShape==kPtFONLL8TeVfeeddown){
    funcPt=new TF1("fFONLL","[0]*x/TMath::Power((1+TMath::Power(x/[1],[3])),[2])",0.,50.);
    funcPt->SetParameters(0.398252, 3.9603, 3.915, 1.51853); // FeedDown
    outFileName.Append("FONLL8ptshapeFeedDown_");
  }else if(fPtShape==kPtFONLL7TeV){
    funcPt=new TF1("fFONLL","[0]*x/TMath::Power((1+TMath::Power(x/[1],[3])),[2])",0.,50.);
    funcPt->SetParameters(0.322643,2.96275,2.30301,2.5);
    outFileName.Append("FONLL7ptshape_");
  }else if(fPtShape==kPtFONLL5TeV){
    funcPt=new TF1("fFONLL","[0]*x/TMath::Power((1+TMath::Power(x/[1],[3])),[2])",0.,50.);
    funcPt->SetParameters(0.302879,2.9750,3.68139,1.68855);
    outFileName.Append("FONLL5ptshape_");
  }else if(fPtShape==kPtPythia7TeV){
    funcPt=new TF1("fFONLL","[0]*x/TMath::Power((1+TMath::Power(x/[1],[3])),[2])",0.,50.);
    funcPt->SetParameters(0.322643,1.94635,1.40463,2.5);
    outFileName.Append("PYTHIA7ptshape_");  
  }else if(fPtShape==kPtFONLL13TeVprompt){
    if(fDDecay==kDplusKpipi){
      histPt = LoadFONLL13TeV_promptDplus();
      outFileName.Append("promptDplus");
    }else if (fDDecay==kDstarD0pi){
      histPt = LoadFONLL13TeV_promptDstar();
      outFileName.Append("promptDstar");
    }else{
      histPt = LoadFONLL13TeV_promptD0();
      outFileName.Append("promptD0");
    }
    outFileName.Append("FONLL13ptshape_");
  }else if (fPtShape==kPtFONLL13TeVfeeddown){
    if (fDDecay==kDstarD0pi){
      histPt = LoadFONLL13TeV_feeddownDstar();
      outFileName.Append("feeddownDstar");
    }else{
      histPt = LoadFONLL13TeV_feeddownD();
      outFileName.Append("feeddownD");
    }
    outFileName.Append("FONLL13ptshape_");
  }else if(fPtShape==kPtPythia13TeVprompt){
    if(fDDecay==kDplusKpipi){
      histPt = LoadPYTHIA13TeV_promptDplus();
      outFileName.Append("promptDplus");
    }else if (fDDecay==kDstarD0pi){
      histPt = LoadPYTHIA13TeV_promptDstar();
      outFileName.Append("promptDstar");
    }else if (fDDecay==kDsKKpi){
      histPt = LoadPYTHIA13TeV_promptDs();
      outFileName.Append("promptDs");
    }else if (fDDecay==kLcpKpi){
      histPt = LoadPYTHIA13TeV_promptLc();
      outFileName.Append("promptLc");
    }else{
      histPt = LoadPYTHIA13TeV_promptD0();
      outFileName.Append("promptD0");
    }
    outFileName.Append("PYTHIA13ptshape_");
  }else if (fPtShape==kPtPythia13TeVfeeddown){
    if(fDDecay==kDplusKpipi){
      histPt = LoadPYTHIA13TeV_feeddownDplus();
      outFileName.Append("feeddownDplus");
    }else if (fDDecay==kDstarD0pi){
      histPt = LoadPYTHIA13TeV_feeddownDstar();
      outFileName.Append("feeddownDstar");
    }else if (fDDecay==kDsKKpi){
      histPt = LoadPYTHIA13TeV_feeddownDs();
      outFileName.Append("feeddownDs");
    }else if (fDDecay==kLcpKpi){
      histPt = LoadPYTHIA13TeV_feeddownLc();
      outFileName.Append("feeddownLc");
    }else{
      histPt = LoadPYTHIA13TeV_feeddownD0();
      outFileName.Append("feeddownD0");
    }
    outFileName.Append("PYTHIA13ptshape_");
  }else{
    funcPt=new TF1("fFlat","pol0",0.,50.);
    funcPt->SetParameter(0,1.);
    outFileName.Append("flatpt_");
  }
  if (funcPt) funcPt->SetNpx(10000);

  TF1*  funcY=0x0;
  if(fYShape==kYFONLL5TeV){
    funcY=new TF1("fsigyfonll5","[0]+((x>0.5)*[1]/sqrt(x))+((x<0.5)*([1]*sqrt(2)))",0.,50);
    funcY->SetParameters(7.22252e-01,5.06914);
    outFileName.Append("FONLLy.root");
  }else if(fYShape==kYFONLLMax5TeV){
    funcY=new TF1("fsigyfonll5","TMath::Min(8.,[0]+[1]/sqrt(x))",0.,50);
    funcY->SetParameters(7.05870e-01,5.11006);
    outFileName.Append("FONLLyMax.root");
  }else if(fYShape==kYFONLLMin5TeV){
    funcY=new TF1("fsigyfonll5","TMath::Min(8.,[0]+[1]/sqrt(x)+[2]/(x*x*x))",0.,50);
    funcY->SetParameters(6.99901e-01,5.27825,1.54156e+01);
    outFileName.Append("FONLLyMin.root");
  }else if(fYShape==kYFONLL13TeV){
    funcY=new TF1("fsigyfonll13","TMath::Min(8.,[0]+[1]/sqrt(x))",0.,50);
    funcY->SetParameters(1.076661,5.845579);
    outFileName.Append("FONLLy.root");
  }else if(fYShape==kYFONLLMax13TeV){
    funcY=new TF1("fsigyfonll13","TMath::Min(8.,[0]+[1]/sqrt(x))",0.,50);
    funcY->SetParameters(1.031292,5.961879);
    outFileName.Append("FONLLyMax.root");
  }else if(fYShape==kYFONLLMin13TeV){
    funcY=new TF1("fsigyfonll13","TMath::Min(8.,[0]+[1]/sqrt(x)+[2]/(x*x*x))",0.,50);
    funcY->SetParameters(1.100997,5.952721,49.171957);
    outFileName.Append("FONLLyMin.root");
  }else{
    outFileName.Append("flaty.root");
  }
  if (funcY) funcY->SetNpx(10000);
  
  TRandom3* gener=new TRandom3(0);
  TLorentzVector* vec=new TLorentzVector();

  for(Int_t itry=0; itry<totTrials; itry++){
    if(itry%100000==0) printf("Event %d\n",itry);
    Float_t ptD = funcPt ? funcPt->GetRandom() : histPt->GetRandom();
    Float_t phiD=gener->Rndm()*2*TMath::Pi();
    Float_t yD=-9999;
    if(fYShape==kFlatY || !funcY) yD=gener->Rndm()*2.-1.; // flat in -1<y<1
    else{
      Float_t sgaus=funcY->Eval(ptD);
      while(1){
	yD=gener->Gaus(0,sgaus);
	if(TMath::Abs(yD)<1) break;
      }
    }
    Float_t px=ptD*TMath::Cos(phiD);
    Float_t py=ptD*TMath::Sin(phiD);
    Float_t mt=TMath::Sqrt(massD*massD+ptD*ptD);
    Float_t pz=mt*TMath::SinH(yD);
    Float_t E=TMath::Sqrt(massD*massD+px*px+py*py+pz*pz);

    // TLorentzVector* vec=new TLorentzVector(px,py,pz,E);
    vec->SetPxPyPzE(px,py,pz,E);
    pdec->Decay(pdgCode,vec);
    array->Clear();
    Int_t nentries = pdec->ImportParticles(array);
    TParticle* dmes=(TParticle*)array->At(0);
    Int_t nDaughters=dmes->GetNDaughters();
    if(fDDecay==kD0Kpi && nDaughters!=2) continue;
    Int_t nPionsInAcc=0;
    Int_t nProtonsInAcc=0;
    Int_t nKaonsInAcc=0;
    Int_t nPions=0;
    Int_t nProtons=0;
    Int_t nKaons=0;
    Int_t idLcResChan=0; // non resonant by default;
    std::vector<Int_t> pdgDauAll{};
    Bool_t isOk=CountPKpi(array,nentries,nPions,nKaons,nProtons,nPionsInAcc,nKaonsInAcc,nProtonsInAcc, idLcResChan, pdgDauAll);
    if(fDDecay==kLcK0Sp && std::find(pdgDauAll.begin(), pdgDauAll.end(), 310) == pdgDauAll.end()) continue;
    if(fDDecay==kLcpiL && std::find(pdgDauAll.begin(), pdgDauAll.end(), 3122) == pdgDauAll.end()) continue;
    if(isOk){
      if(nPions==nPionDau && nKaons==nKaonDau && nProtons==nProtonDau){
	hPtVsYGen->Fill(ptD,yD);
	if(TMath::Abs(yD)<0.5){
	  hPtVsYGenLimAcc->Fill(ptD,yD);
	  hPtVsYGenLimAccLcpKpi[idLcResChan]->Fill(ptD,yD); 
	}
	if(IsInFiducialAcceptance(ptD,yD)){	  
	  if(nPionsInAcc==nPionDau && nKaonsInAcc==nKaonDau && nProtonsInAcc==nProtonDau){ 
	    hPtVsYGenAcc->Fill(ptD,yD);
	    hPtVsYGenAccLcpKpi[idLcResChan]->Fill(ptD,yD); 
	  }
	}
      }
    }
    // delete vec;
  }

  TH1D* hPtGenAcc=(TH1D*)hPtVsYGenAcc->ProjectionX("hPtGenAcc"); 
  hPtGenAcc->GetYaxis()->SetTitle("Entries");
  TH1D* hPtGenLimAcc=(TH1D*)hPtVsYGenLimAcc->ProjectionX("hPtGenLimAcc"); 
  hPtGenLimAcc->GetYaxis()->SetTitle("Entries");
  hPtGenAcc->Sumw2();
  hPtGenLimAcc->Sumw2();
  TH1D* hAccVsPt=AliVertexingHFUtils::ComputeGenAccOverGenLimAcc(hPtVsYGenAcc,hPtVsYGenLimAcc);
  hAccVsPt->GetYaxis()->SetTitle("Acceptance");
  hAccVsPt->SetStats(0);

  TCanvas* c2d=new TCanvas("c2d","Pt vs y",1200,600);
  c2d->Divide(3,1);
  c2d->cd(1);
  hPtVsYGen->Draw("colz");
  c2d->cd(2);
  hPtVsYGenLimAcc->Draw("colz");
  c2d->cd(3);
  hPtVsYGenAcc->Draw("colz");

  TCanvas* c1d=new TCanvas("c1d","Acceptance",1200,600);
  c1d->Divide(2,1);
  c1d->cd(1);
  hPtGenLimAcc->Draw("");
  Double_t ymax=1.2*TMath::Max(hPtGenLimAcc->GetMaximum(),hPtGenAcc->GetMaximum());
  hPtGenLimAcc->SetMaximum(ymax);
  gPad->Update();
  TPaveStats *st1=(TPaveStats*)hPtGenLimAcc->GetListOfFunctions()->FindObject("stats");
  st1->SetY1NDC(0.71);
  st1->SetY2NDC(0.9);
  hPtGenAcc->SetLineColor(kRed+1);
  hPtGenAcc->Draw("sames");
  gPad->Update();
  TPaveStats *st2=(TPaveStats*)hPtGenAcc->GetListOfFunctions()->FindObject("stats");
  st2->SetY1NDC(0.51);
  st2->SetY2NDC(0.7);
  st2->SetTextColor(hPtGenAcc->GetLineColor());
  gPad->Modified();
  c1d->cd(2);
  hAccVsPt->Draw();


  TH1D* hPtGenAccLcpKpi[4]={0x0,0x0,0x0,0x0};
  TH1D* hPtGenLimAccLcpKpi[4]={0x0,0x0,0x0,0x0};
  TH1D* hAccVsPtLcpKpi[4]={0x0,0x0,0x0,0x0};
  if(fDDecay==kLcpKpi){
    Int_t nentr[4];
    Int_t nSum=0;
    Int_t nTot=hPtGenLimAcc->GetEntries();
    printf("Lc decay modes fractions: ");
    for(Int_t ich=0; ich<4; ich++){
      hPtGenAccLcpKpi[ich]=(TH1D*)hPtVsYGenAccLcpKpi[ich]->ProjectionX(Form("hPtGenAcc%s",lcChan[ich].Data()));
      hPtGenAccLcpKpi[ich]->GetYaxis()->SetTitle("Entries");
      hPtGenAccLcpKpi[ich]->Sumw2();
      hPtGenLimAccLcpKpi[ich]=(TH1D*)hPtVsYGenLimAccLcpKpi[ich]->ProjectionX(Form("hPtGenLimAcc%s",lcChan[ich].Data()));
      hPtGenLimAccLcpKpi[ich]->GetYaxis()->SetTitle("Entries");
      hPtGenLimAccLcpKpi[ich]->Sumw2();
      hAccVsPtLcpKpi[ich]=(TH1D*)hPtGenAccLcpKpi[ich]->Clone(Form("hAccVsPt%s",lcChan[ich].Data()));
      hAccVsPtLcpKpi[ich]->Divide(hPtGenAccLcpKpi[ich],hPtGenLimAccLcpKpi[ich],1.,1.,"B");
      hAccVsPtLcpKpi[ich]->GetYaxis()->SetTitle("Acceptance");
      hAccVsPtLcpKpi[ich]->SetStats(0);
      nentr[ich]=hPtGenLimAccLcpKpi[ich]->GetEntries();
      nSum+=nentr[ich];
      printf(" %s = %.3f,  ",lcChan[ich].Data(),(Float_t)nentr[ich]/(Float_t)nTot);
    }
    printf("Check   = %.3f\n",(Float_t)nSum/(Float_t)nTot);
  }

  TFile* outfil=new TFile(outFileName.Data(),"recreate");
  hPtVsYGen->Write();
  hPtVsYGenLimAcc->Write();
  hPtVsYGenAcc->Write();
  hPtGenLimAcc->Write();
  hPtGenAcc->Write();
  hAccVsPt->Write();
  if(fDDecay==kLcpKpi){
    for(Int_t ich=0; ich<4; ich++){
      if(hPtGenAccLcpKpi[ich]) hPtGenAccLcpKpi[ich]->Write();
      if(hPtGenLimAccLcpKpi[ich]) hPtGenLimAccLcpKpi[ich]->Write();
      if(hAccVsPtLcpKpi[ich]) hAccVsPtLcpKpi[ich]->Write();
    }
  }
  outfil->Close();

}

//___________________________________________________
Bool_t IsInFiducialAcceptance(Double_t pt, Double_t y){
  // check fiducial acceptance

  if(fOptionYFiducial==kFixedY){
    if(TMath::Abs(y) > fYMaxFidAccCut) return kFALSE;
    else return kTRUE;
  }

  if(pt > 5.) {
    if (TMath::Abs(y) > 0.8) return kFALSE;
  } else {
    Double_t maxFiducialY = -0.2/15*pt*pt+1.9/15*pt+0.5; 
    Double_t minFiducialY = 0.2/15*pt*pt-1.9/15*pt-0.5;		
    if (y < minFiducialY || y > maxFiducialY) return kFALSE;
  }

  return kTRUE;
}


//___________________________________________________
Bool_t CountPKpi(TClonesArray *array, Int_t nentries, Int_t &nPions, Int_t &nKaons, Int_t &nProtons, Int_t &nPionsInAcc, Int_t &nKaonsInAcc, Int_t &nProtonsInAcc, Int_t &idLcResChan, std::vector<Int_t> &pdgDauAll){
  // count K and pi in Acc

  TParticle* dmes=(TParticle*)array->At(0);
  Double_t sumPx=0;
  Double_t sumPy=0;
  Double_t sumPz=0;
  
  for(int j=0; j<nentries; j++){
    TParticle * o = (TParticle*)array->At(j);
    Int_t pdgdau=TMath::Abs(o->GetPdgCode());
    Int_t pdgMother = -1;
    if(fDDecay==kDstarD0pi && j>0)
    {
        Int_t idxMother = o->GetFirstMother()-1;
        TParticle *moth = (TParticle*)array->At(idxMother);
        pdgMother = TMath::Abs(moth->GetPdgCode());
    }
    pdgDauAll.push_back(pdgdau);
    if(fDebugLevel>0) printf("%d ",pdgdau);
    if(pdgdau==130) {
      if(fDebugLevel>0) printf("K0 dacaying into K0L\n");
      return kFALSE;
    }
    Float_t ptdau=TMath::Sqrt(o->Px()*o->Px()+o->Py()*o->Py());      
    Float_t etadau=o->Eta();
    if(pdgdau==211){ 
      nPions++;
      sumPx+=o->Px();
      sumPy+=o->Py();
      sumPz+=o->Pz();
    }
    if(pdgdau==321){ 
      nKaons++;
      sumPx+=o->Px();
      sumPy+=o->Py();
      sumPz+=o->Pz();
    }
    if(pdgdau==2212){ 
      nProtons++;
      sumPx+=o->Px();
      sumPy+=o->Py();
      sumPz+=o->Pz();
    }
    if(pdgdau==313) idLcResChan=2; //K*0 
    else if(pdgdau==2224) idLcResChan=3; //Delta++
    else if(pdgdau==3124) idLcResChan=1;  //Lambda1520
    if(TMath::Abs(etadau)<fEtaMaxDau && (ptdau>fPtMinDau || (pdgMother==413 && pdgdau==211 && ptdau>fPtMinDauSoftPi))){
      if(pdgdau==211) nPionsInAcc++;
      if(pdgdau==321) nKaonsInAcc++;
      if(pdgdau==2212) nProtonsInAcc++;
    }
  }
  if(fDebugLevel>0) printf("\n");
  if(TMath::Abs(sumPx-dmes->Px())>0.001 ||
     TMath::Abs(sumPy-dmes->Py())>0.001 ||
     TMath::Abs(sumPz-dmes->Pz())>0.001){
    printf("Momentum conservation violation\n");
    return kFALSE;
  }
  return kTRUE;
}



//___________________________________________________
TH1D* LoadFONLL13TeV_promptD0()
{
  TH1D *hFONLL13 = new TH1D("hFONLL13TeV_D0", "", 80, 0., 40.);
  Float_t val[80] = {
    1.4686e+08, 3.9542e+08, 5.4901e+08, 5.2166e+08, 4.1083e+08, 2.9968e+08, 2.1299e+08, 1.5057e+08, 1.0701e+08, 7.6919e+07,
    5.6121e+07, 4.1546e+07, 3.1184e+07, 2.3715e+07, 1.8253e+07, 1.4206e+07, 1.1175e+07, 8.8774e+06, 7.1169e+06, 5.7544e+06,
    4.6899e+06, 3.8509e+06, 3.1841e+06, 2.6499e+06, 2.2189e+06, 1.8687e+06, 1.5823e+06, 1.3467e+06, 1.1516e+06, 9.8933e+05,
    8.5356e+05, 7.3942e+05, 6.4302e+05, 5.6122e+05, 4.9154e+05, 4.3193e+05, 3.8074e+05, 3.3662e+05, 2.9846e+05, 2.6535e+05,
    2.3653e+05, 2.1136e+05, 1.8932e+05, 1.6997e+05, 1.5293e+05, 1.3789e+05, 1.2458e+05, 1.1277e+05, 1.0227e+05, 9.2913e+04,
    8.4560e+04, 7.7087e+04, 7.0388e+04, 6.4371e+04, 5.8956e+04, 5.4074e+04, 4.9667e+04, 4.5680e+04, 4.2068e+04, 3.8791e+04,
    3.5813e+04, 3.3103e+04, 3.0633e+04, 2.8380e+04, 2.6321e+04, 2.4436e+04, 2.2710e+04, 2.1127e+04, 1.9673e+04, 1.8336e+04,
    1.7106e+04, 1.5972e+04, 1.4926e+04, 1.3961e+04, 1.3069e+04, 1.2243e+04, 1.1479e+04, 1.0771e+04, 1.0113e+04, 9.5031e+03
  };
  for (Int_t ibin=0; ibin<80; ++ibin) hFONLL13->SetBinContent(ibin+1, val[ibin]);

  return hFONLL13;
}



//___________________________________________________
TH1D* LoadFONLL13TeV_promptDplus()
{
  TH1D *hFONLL13 = new TH1D("hFONLL13TeV_Dplus", "", 80, 0., 40.);
  Float_t val[80] = {
    1.5242e+08, 3.9396e+08, 5.3767e+08, 5.1263e+08, 4.0706e+08, 2.9926e+08, 2.1414e+08, 1.5230e+08, 1.0879e+08, 7.8507e+07,
    5.7460e+07, 4.2651e+07, 3.2093e+07, 2.4462e+07, 1.8865e+07, 1.4708e+07, 1.1588e+07, 9.2185e+06, 7.3998e+06, 5.9900e+06,
    4.8871e+06, 4.0167e+06, 3.3240e+06, 2.7686e+06, 2.3200e+06, 1.9552e+06, 1.6566e+06, 1.4107e+06, 1.2070e+06, 1.0374e+06,
    8.9550e+05, 7.7610e+05, 6.7519e+05, 5.8953e+05, 5.1652e+05, 4.5404e+05, 4.0037e+05, 3.5409e+05, 3.1405e+05, 2.7929e+05,
    2.4902e+05, 2.2258e+05, 1.9943e+05, 1.7909e+05, 1.6117e+05, 1.4535e+05, 1.3135e+05, 1.1892e+05, 1.0787e+05, 9.8024e+04,
    8.9230e+04, 8.1359e+04, 7.4302e+04, 6.7963e+04, 6.2256e+04, 5.7112e+04, 5.2465e+04, 4.8261e+04, 4.4452e+04, 4.0996e+04,
    3.7854e+04, 3.4995e+04, 3.2390e+04, 3.0012e+04, 2.7838e+04, 2.5849e+04, 2.4027e+04, 2.2355e+04, 2.0820e+04, 1.9408e+04,
    1.8108e+04, 1.6910e+04, 1.5805e+04, 1.4785e+04, 1.3842e+04, 1.2969e+04, 1.2161e+04, 1.1411e+04, 1.0716e+04, 1.0070e+04
  };
  for (Int_t ibin=0; ibin<80; ++ibin) hFONLL13->SetBinContent(ibin+1, val[ibin]);

  return hFONLL13;
}



//___________________________________________________
TH1D* LoadFONLL13TeV_promptDstar()
{
  TH1D *hFONLL13 = new TH1D("hFONLL13TeV_Dstar", "", 80, 0., 40.);
  Float_t val[80] = {
    1.2433e+08, 3.4512e+08, 5.0662e+08, 5.1020e+08, 4.2016e+08, 3.1661e+08, 2.3064e+08, 1.6632e+08, 1.2007e+08, 8.7340e+07,
    6.4329e+07, 4.8000e+07, 3.6287e+07, 2.7772e+07, 2.1492e+07, 1.6808e+07, 1.3278e+07, 1.0589e+07, 8.5178e+06, 6.9084e+06,
    5.6462e+06, 4.6479e+06, 3.8519e+06, 3.2124e+06, 2.6951e+06, 2.2738e+06, 1.9285e+06, 1.6437e+06, 1.4076e+06, 1.2108e+06,
    1.0459e+06, 9.0711e+05, 7.8968e+05, 6.8993e+05, 6.0484e+05, 5.3197e+05, 4.6933e+05, 4.1529e+05, 3.6850e+05, 3.2786e+05,
    2.9246e+05, 2.6152e+05, 2.3441e+05, 2.1058e+05, 1.8958e+05, 1.7104e+05, 1.5461e+05, 1.4003e+05, 1.2706e+05, 1.1550e+05,
    1.0517e+05, 9.5920e+04, 8.7626e+04, 8.0172e+04, 7.3461e+04, 6.7409e+04, 6.1940e+04, 5.6992e+04, 5.2508e+04, 4.8437e+04,
    4.4736e+04, 4.1367e+04, 3.8296e+04, 3.5492e+04, 3.2929e+04, 3.0583e+04, 2.8433e+04, 2.6460e+04, 2.4648e+04, 2.2981e+04,
    2.1446e+04, 2.0032e+04, 1.8727e+04, 1.7522e+04, 1.6407e+04, 1.5376e+04, 1.4420e+04, 1.3535e+04, 1.2713e+04, 1.1950e+04
  };
  for (Int_t ibin=0; ibin<80; ++ibin) hFONLL13->SetBinContent(ibin+1, val[ibin]);

  return hFONLL13;
}



//___________________________________________________
TH1D* LoadFONLL13TeV_feeddownD()
{ // B->D with B.R=1
  TH1D *hFONLL13 = new TH1D("FONLL13TeV_feeddownD", "", 80, 0., 40.);
  Float_t val[80] = {
    1.0310e+07, 2.6790e+07, 3.4480e+07, 3.4430e+07, 3.0200e+07, 2.4740e+07, 1.9600e+07, 1.5300e+07, 1.1880e+07, 9.2260e+06,
    7.1950e+06, 5.6470e+06, 4.4650e+06, 3.5580e+06, 2.8570e+06, 2.3120e+06, 1.8860e+06, 1.5490e+06, 1.2810e+06, 1.0660e+06,
    8.9260e+05, 7.5170e+05, 6.3650e+05, 5.4160e+05, 4.6310e+05, 3.9770e+05, 3.4300e+05, 2.9700e+05, 2.5820e+05, 2.2520e+05,
    1.9710e+05, 1.7310e+05, 1.5240e+05, 1.3460e+05, 1.1920e+05, 1.0590e+05, 9.4290e+04, 8.4160e+04, 7.5290e+04, 6.7500e+04,
    6.0650e+04, 5.4610e+04, 4.9270e+04, 4.4530e+04, 4.0320e+04, 3.6580e+04, 3.3240e+04, 3.0250e+04, 2.7570e+04, 2.5170e+04,
    2.3020e+04, 2.1070e+04, 1.9320e+04, 1.7740e+04, 1.6310e+04, 1.5010e+04, 1.3830e+04, 1.2760e+04, 1.1790e+04, 1.0900e+04,
    1.0090e+04, 9.3520e+03, 8.6760e+03, 8.0560e+03, 7.4890e+03, 6.9670e+03, 6.4880e+03, 6.0470e+03, 5.6410e+03, 5.2670e+03,
    4.9220e+03, 4.6030e+03, 4.3080e+03, 4.0350e+03, 3.7830e+03, 3.5480e+03, 3.3310e+03, 3.1290e+03, 2.9410e+03, 2.7670e+03
  };
  for (Int_t ibin=0; ibin<80; ++ibin) hFONLL13->SetBinContent(ibin+1, val[ibin]);

  return hFONLL13;
}



//___________________________________________________
TH1D* LoadFONLL13TeV_feeddownDstar()
{ // B->D* with B.R=1
  TH1D *hFONLL13 = new TH1D("FONLL13TeV_feeddownDstar", "", 80, 0., 40.);
  Float_t val[80] = {
    9.5260e+06, 2.5070e+07, 3.2890e+07, 3.3540e+07, 3.0010e+07, 2.5000e+07, 2.0090e+07, 1.5860e+07, 1.2430e+07, 9.7280e+06,
    7.6360e+06, 6.0240e+06, 4.7820e+06, 3.8240e+06, 3.0800e+06, 2.5000e+06, 2.0430e+06, 1.6810e+06, 1.3930e+06, 1.1610e+06,
    9.7390e+05, 8.2120e+05, 6.9610e+05, 5.9300e+05, 5.0750e+05, 4.3620e+05, 3.7650e+05, 3.2620e+05, 2.8370e+05, 2.4760e+05,
    2.1680e+05, 1.9050e+05, 1.6780e+05, 1.4830e+05, 1.3140e+05, 1.1680e+05, 1.0400e+05, 9.2870e+04, 8.3100e+04, 7.4530e+04,
    6.6990e+04, 6.0330e+04, 5.4440e+04, 4.9220e+04, 4.4580e+04, 4.0440e+04, 3.6760e+04, 3.3460e+04, 3.0510e+04, 2.7860e+04,
    2.5470e+04, 2.3330e+04, 2.1390e+04, 1.9640e+04, 1.8060e+04, 1.6630e+04, 1.5320e+04, 1.4140e+04, 1.3060e+04, 1.2080e+04,
    1.1190e+04, 1.0370e+04, 9.6190e+03, 8.9330e+03, 8.3050e+03, 7.7270e+03, 7.1970e+03, 6.7080e+03, 6.2580e+03, 5.8440e+03,
    5.4610e+03, 5.1080e+03, 4.7810e+03, 4.4790e+03, 4.1980e+03, 3.9390e+03, 3.6970e+03, 3.4740e+03, 3.2650e+03, 3.0720e+03
  };
  for (Int_t ibin=0; ibin<80; ++ibin) hFONLL13->SetBinContent(ibin+1, val[ibin]);

  return hFONLL13;
}



//___________________________________________________
TH1D* LoadPYTHIA13TeV_promptD0()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13TeV_D0", "", 40, 0., 40.);
  Float_t val[40] = {
    2617743, 3763836, 2235903, 1140807, 587707, 317881, 180756, 107762, 67299,42882,
    28550, 19690, 13817, 9754, 7205, 5299, 4100, 3177, 2386,1822,
    1484, 1248, 933, 798, 639, 534, 435, 405, 304,260,
    210, 196, 161, 143, 139, 99, 89, 80, 66,63
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}



//___________________________________________________
TH1D* LoadPYTHIA13TeV_promptDplus()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13TeV_Dplus", "", 40, 0., 40.);
  Float_t val[40] = {
    1192379, 1736283, 1047485, 540499, 281523, 152153, 87150, 52200, 32709, 20991,
    13986, 9611, 6832, 4705, 3484, 2650, 2001, 1518, 1197, 924,
    735, 576, 497, 402, 292, 261, 214, 173, 169, 150,
    106, 102, 95, 70, 67, 47, 53, 46, 35, 35
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}



//___________________________________________________
TH1D* LoadPYTHIA13TeV_promptDstar()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13TeV_Dstar", "", 40, 0., 40.);
  Float_t val[40] = {
    922985, 1419749, 904587, 484407, 257862, 142386, 82177, 49839, 31103, 20422,
    13493, 9272, 6549, 4746, 3344, 2575, 1959, 1493, 1161, 935,
    672, 585, 456, 415, 328, 260, 199, 180, 161, 134,
    109, 97, 103, 72, 63, 56, 40, 46, 38, 27
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}



//___________________________________________________
TH1D* LoadPYTHIA13TeV_promptDs()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13TeV_Ds", "", 40, 0., 40.);
  Float_t val[40] = {
    346381, 519143, 320488, 167944, 87484, 47325, 26932, 16376, 10058, 6527,
    4347, 3041, 2112, 1521, 1069, 849, 609, 468, 359, 275,
    226, 197, 152, 124, 111, 90, 63, 60, 52, 46,
    39, 35, 26, 29, 16, 20, 10, 13, 18, 9
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}


//___________________________________________________
TH1D* LoadPYTHIA13TeV_promptLc()
{
  // coarse binning (1 GeV/c)
  /*TH1D *hPYTHIA13 = new TH1D("hPYTHIA13TeV_Lc", "", 40, 0., 40.);
  Float_t val[40] = {
    307191, 504151, 400086, 264143, 161012, 96421, 57765, 35876, 22207, 14852, 9760, 6853, 4814, 3453, 2594, 1838, 1457, 1091, 850, 702, 593, 402, 366, 284, 250, 213, 171, 127, 128, 109, 89, 72, 54, 48, 42, 34, 30, 34, 39, 20
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);*/

  // fine binning (0.2 GeV/c)
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13TeV_Lc", "", 200, 0., 40.);
  Float_t val[200] = {
    14916, 43781, 67845, 84755, 95894, 101546, 103271, 103306, 99662, 96366, 91138, 86070, 80051, 74407, 68420, 63088, 57682, 52478, 47917, 42978, 39055, 35664, 31415, 28898, 25980, 23237, 21098, 19134, 17419, 15533, 13903, 12743, 11387, 10418, 9314, 8665, 7697, 7079, 6486, 5949, 5237, 4785, 4378, 4130, 3677, 3532, 3150, 2950, 2764, 2456, 2245, 2022, 2007, 1781, 1705, 1554, 1476, 1395, 1316, 1112, 1096, 1023, 937, 888, 870, 787, 723, 698, 627, 618, 569, 556, 510, 478, 481, 401, 390, 370, 355, 322, 358, 308, 275, 258, 258, 246, 220, 227, 194, 204, 188, 185, 171, 157, 149, 144, 151, 143, 138, 126, 132, 120, 113, 105, 123, 89, 85, 85, 67, 76, 79, 73, 65, 74, 75, 62, 63, 57, 53, 49, 45, 37, 62, 53, 53, 45, 37, 41, 43, 47, 47, 33, 38, 30, 23, 32, 24, 25, 16, 30, 26, 22, 28, 24, 28, 25, 21, 24, 21, 18, 16, 18, 28, 15, 12, 15, 16, 16, 13, 12, 10, 9, 12, 14, 9, 10, 8, 9, 8, 13, 11, 6, 16, 3, 6, 7, 9, 5, 4, 9, 3, 8, 5, 6, 8, 5, 9, 8, 9, 3, 10, 10, 6, 11, 2, 6, 3, 5, 3, 3
  };
  for (Int_t ibin=0; ibin<200; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}


//___________________________________________________
TH1D* LoadPYTHIA13TeV_feeddownD0()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13_feeddownD0", "", 40, 0., 40.);
  Float_t val[40] = {
    1814030, 2939561, 2059721, 1197596, 684753, 397435, 237432, 146442, 93029, 60222,
    40940, 28861, 19992, 14344, 10602, 7742, 5781, 4395, 3421, 2732,
    2216, 1763, 1395, 1208, 888, 735, 599, 509, 418, 374,
    302, 281, 242, 200, 174, 158, 133, 104, 98, 88
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}



//___________________________________________________
TH1D* LoadPYTHIA13TeV_feeddownDplus()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13_feeddownDplus", "", 40, 0., 40.);
  Float_t val[40] = {
    779389, 1268000, 899979, 527442, 302782, 177610, 105677, 66601, 42008, 27618,
    18802, 13024, 8975, 6530, 4773, 3495, 2708, 2020, 1634, 1212,
    1017, 809, 665, 507, 389, 355, 285, 250, 218, 180,
    143, 137, 106, 85, 71, 64, 57, 43, 50, 47
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}



//___________________________________________________
TH1D* LoadPYTHIA13TeV_feeddownDstar()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13_feeddownDstar", "", 40, 0., 40.);
  Float_t val[40] = {
    665735, 1144214, 856647, 523704, 309061, 183569, 111994, 70187, 45266, 29601,
    20079, 14042, 10338, 7184, 5234, 4002, 2935, 2301, 1788, 1349,
    1088, 895, 748, 591, 480, 378, 302, 263, 216, 182,
    172, 122, 133, 100, 92, 87, 65, 50, 42, 45
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}



//___________________________________________________
TH1D* LoadPYTHIA13TeV_feeddownDs()
{
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13_feeddownDs", "", 40, 0., 40.);
  Float_t val[40] = {
    377925, 686435, 518802, 318819, 189396, 113651, 68963, 43048, 27776, 18177,
    12356, 8643, 6100, 4512, 3170, 2479, 1835, 1424, 1086, 825,
    659, 490, 465, 361, 308, 246, 181, 170, 117, 111,
    112, 91, 69, 66, 54, 57, 39, 33, 31, 25
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}


//___________________________________________________
TH1D* LoadPYTHIA13TeV_feeddownLc()
{
  // coarse binning (1 GeV/c)
  /*TH1D *hPYTHIA13 = new TH1D("hPYTHIA13_feeddownLc", "", 40, 0., 40.);
  Float_t val[40] = {
    274999, 525408, 449454, 306100, 193183, 119904, 75652, 47853, 31371, 20955, 14419, 9906, 7140, 5232, 3752, 2808, 2125, 1672, 1305, 994, 800, 659, 540, 446, 354, 301, 246, 209, 173, 125, 129, 93, 107, 83, 78, 54, 48, 46, 42, 38
  };
  for (Int_t ibin=0; ibin<40; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);*/

  // fine binning (0.2 GeV/c)
  TH1D *hPYTHIA13 = new TH1D("hPYTHIA13_feeddownLc", "", 200, 0., 40.);
  Float_t val[200] = {
    12725, 37018, 58333, 76501, 90422, 99701, 105650, 107472, 107723, 104862, 101013, 95436, 90881, 84179, 77945, 72814, 66631, 60966, 55052, 50637, 46183, 42126, 38369, 34844, 31661, 28670, 26343, 23685, 21596, 19610, 18195, 16580, 14852, 13551, 12474, 11219, 10293, 9498, 8785, 8058, 7335, 6842, 6183, 5762, 5249, 4956, 4482, 4147, 3808, 3562, 3328, 3070, 2831, 2694, 2496, 2269, 2131, 2015, 1770, 1721, 1581, 1530, 1390, 1380, 1259, 1184, 1111, 1079, 980, 878, 864, 798, 741, 646, 703, 661, 564, 575, 524, 484, 442, 457, 446, 401, 379, 375, 342, 348, 328, 279, 271, 267, 280, 241, 246, 226, 207, 207, 181, 173, 168, 155, 169, 160, 148, 136, 128, 151, 132, 112, 116, 114, 110, 96, 104, 98, 92, 98, 78, 80, 69, 72, 75, 62, 76, 72, 61, 65, 59, 44, 48, 52, 51, 61, 34, 48, 51, 33, 34, 43, 43, 23, 39, 30, 38, 22, 28, 29, 25, 21, 23, 37, 19, 24, 26, 24, 18, 15, 18, 18, 23, 20, 20, 25, 19, 20, 22, 17, 10, 14, 19, 13, 14, 13, 19, 7, 16, 8, 9, 14, 12, 8, 11, 11, 6, 7, 9, 15, 10, 5, 8, 5, 17, 6, 6, 10, 9, 7, 6, 6
  };
  for (Int_t ibin=0; ibin<200; ++ibin) hPYTHIA13->SetBinContent(ibin+1, val[ibin]);

  return hPYTHIA13;
}
