#include "AliGFWCumulant.h"

AliGFWCumulant::AliGFWCumulant():
  fQvector(0),
  fUsed(kBlank),
  fNEntries(-1),
  fN(1),
  fPow(1),
  fPt(1),
  fInitialized(kFALSE)
{
};

AliGFWCumulant::~AliGFWCumulant()
{
  //printf("Destructor (?) for some reason called?\n");
  //DestroyComplexVectorArray();
};
void AliGFWCumulant::FillArray(Double_t eta, Int_t ptin, Double_t phi, Double_t weight) {
  if(!fInitialized)
    CreateComplexVectorArray(1,1,1);
  if(fPt==1) ptin=0; //If one bin, then just fill it straight; otherwise, if ptin is out-of-range, do not fill
  else if(ptin<0 || ptin>=fPt) return;

  for(Int_t lN = 0; lN<fN; lN++) {
    Double_t lSin = TMath::Sin(lN*phi); //No need to recalculate for each power
    Double_t lCos = TMath::Cos(lN*phi); //No need to recalculate for each power
    for(Int_t lPow=0; lPow<PW(lN); lPow++) {
      Double_t lPrefactor = TMath::Power(weight, lPow); //Dont calculate it twice; multiplication is cheaper that power
      Double_t qsin = lPrefactor * lSin;
      Double_t qcos = lPrefactor * lCos;
      fQvector[ptin][lN][lPow](fQvector[ptin][lN][lPow].Re()+qcos,fQvector[ptin][lN][lPow].Im()+qsin);//+=TComplex(qcos,qsin);
    };
  };
  Inc();
};
void AliGFWCumulant::ResetQs() {
  if(!fNEntries) return; //If 0 entries, then no need to reset. Otherwise, if -1, then just initialized and need to set to 0.
  for(Int_t lN=0;lN<fN;lN++)
    for(Int_t lPow=0;lPow<PW(lN);lPow++) {
      for(Int_t i=0; i<fPt; i++) {
	       fQvector[i][lN][lPow](0.,0.);
      };
    };
  fNEntries=0;
};
void AliGFWCumulant::DestroyComplexVectorArray() {
  if(!fInitialized) return;
  for(Int_t l_n = 0; l_n<fN; l_n++) {
    for(Int_t i=0;i<fPt;i++) {
      delete [] fQvector[i][l_n];
    };
  };
  for(Int_t i=0;i<fPt;i++) {
    delete [] fQvector[i];
  };
  delete [] fQvector;
  fInitialized=kFALSE;
  fNEntries=-1;
};

void AliGFWCumulant::CreateComplexVectorArray(Int_t N, Int_t Pow, Int_t Pt) {
  DestroyComplexVectorArray();
  vector<Int_t> pwv;
  for(Int_t i=0;i<N;i++) pwv.push_back(Pow);
  CreateComplexVectorArrayVarPower(N,pwv,Pt);
};
void AliGFWCumulant::CreateComplexVectorArrayVarPower(Int_t N, vector<Int_t> PowVec, Int_t Pt) {
  DestroyComplexVectorArray();
  fN=N;
  fPow=0;
  fPt=Pt;
  fPowVec = PowVec;
  fQvector = new TComplex**[fPt];
  for(Int_t i=0;i<fPt;i++) {
    fQvector[i] = new TComplex*[fN];
  };
  for(Int_t l_n=0;l_n<fN;l_n++) {
    for(Int_t i=0;i<fPt;i++) {
      fQvector[i][l_n] = new TComplex[PW(l_n)];
    };
  };
  ResetQs();
  fInitialized=kTRUE;
};
TComplex AliGFWCumulant::Vec(Int_t n, Int_t p, Int_t ptbin) {
  if(!fInitialized) return 0;
  if(ptbin>=fPt || ptbin<0) ptbin=0;
  if(n>=0) return fQvector[ptbin][n][p];
  return TComplex::Conjugate(fQvector[ptbin][-n][p]);
};
