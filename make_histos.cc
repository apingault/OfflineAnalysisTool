#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "THStack.h"
#include "TH1S.h"
#include "TH1F.h"
#include "TH2S.h"
#include "TH2F.h"
#include "TH3S.h"
#include "TH3F.h"
#include "THistPainter.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TGraphPainter.h"

#include "include/RPCGeometry.h"
#include "include/MsgSvc.h"

using namespace std;

void make_histos(string fName, int WINDOW_WIDTH, int SIGNAL_LOW, int SIGNAL_HIGH){
    ifstream SortedFile(fName.c_str(),ios::in);

    if ( fName.substr(fName.find_last_of(".")) == ".dat" )
        fName = fName.erase(fName.find_last_of("."));
    unsigned EndOfPath = fName.find_last_of("/")+1;

    string pathName = fName.substr(0,EndOfPath);
    string outputName = fName.substr(fName.find("SORTED_")+7);

    string ResultName = pathName + outputName + ".csv";
    ResultName.insert(EndOfPath,"RESULT_");
    ofstream ResultFile(ResultName.c_str(),ios::out);

    string fNameROOT = pathName + outputName + ".root";
    fNameROOT.insert(EndOfPath,"ROOT_");
    TFile ResultROOT(fNameROOT.c_str(),"RECREATE");

    //********************************************************************

    if(SortedFile){
        TH1S* StripProfile = new TH1S("StripProfile","Strip profile",128,-0.5,127.5);
        StripProfile->SetXTitle("Strip");
        StripProfile->SetYTitle("# of events");

        TH1F* TimeProfile = new TH1F("TimeProfile","Arrival time profile",WINDOW_WIDTH,0,WINDOW_WIDTH);
        TimeProfile->SetXTitle("Time [ns]");
        TimeProfile->SetYTitle("# of events");

        TH2F* TDCProfile = new TH2F("TDCProfile","TDC map",128,-0.5,127.5,WINDOW_WIDTH,0,WINDOW_WIDTH);
        TDCProfile->SetXTitle("Strip");
        TDCProfile->SetYTitle("Time [ns]");

        TH1S* HitMultiplicity = new TH1S("HitMultiplicity","Hit multiplicity",31,-0.5,30.5);
        HitMultiplicity->SetXTitle("Multiplicity");
        HitMultiplicity->SetYTitle("# of events");

        TH1S* ClusterMultiplicity = new TH1S("ClusterMultiplicity","Cluster multiplicity",21,-0.5,20.5);
        ClusterMultiplicity->SetXTitle("Multiplicity");
        ClusterMultiplicity->SetYTitle("# of events");

        TH1S* ClusterSize = new TH1S("ClusterSize","Cluster size",16,0.5,16.5);
        ClusterSize->SetXTitle("Multiplicity");
        ClusterSize->SetYTitle("# of events");

        TH1S* ClusterPosition = new TH1S("ClusterPosition","Cluster position profile",128,-0.5,127.5);
        ClusterPosition->SetXTitle("Strip");
        ClusterPosition->SetYTitle("# of events");

        TH1F* ClusterTime = new TH1F("ClusterTime","Cluster arrival time profile",WINDOW_WIDTH,0,WINDOW_WIDTH);
        ClusterTime->SetXTitle("Time [ns]");
        ClusterTime->SetYTitle("# of events");

        TH2F* ClusterProfile = new TH2F("ClusterProfile","Cluster map",128,-0.5,127.5,WINDOW_WIDTH,0,WINDOW_WIDTH);
        ClusterProfile->SetXTitle("Strip");
        ClusterProfile->SetYTitle("Time [ns]");

        TH1S* Efficiency = new TH1S("Efficiency","Detection efficiency",2,-0.5,1.5);
        Efficiency->SetXTitle("(Not detected/Detected)");
        Efficiency->SetYTitle("# of events");

        while(SortedFile.good()){
            int Event = 0;
            int nClusters = -1;

            SortedFile >> Event >> nClusters;

            int cMultiplicity = 0;
            int hMultiplicity = 0;

            if(Event == 0 && nClusters == -1){
                cout << "End of file.\n";
                break;
            } else if(nClusters == 0){
                HitMultiplicity->Fill(0);
                ClusterMultiplicity->Fill(0);
                Efficiency->Fill(0);
            } else {
                for(int c = 0; c < nClusters; c++){
                    int Cluster = 0;
                    int cSize = 0;

                    SortedFile >> Cluster >> cSize;

                    if(Cluster == 0 || cSize == 0){
                        printf("Error : problem with cluster %d on event %d\n",c,Event);
                        exit(EXIT_FAILURE);
                    }

                    float cPosition = 0;
                    float cTime = 0;

                    for(int h = 0; h < cSize; h++){
                        int channel = -1;
                        float time = -1;

                        SortedFile >> channel >> time;

                        if(channel == -1 || time == -1){
                            printf("Error : problem with hit %d of cluster %d on event %d\n",h,c,Event);
                            exit(EXIT_FAILURE);
                        }
                        cPosition += channel;
                        if(cTime == 0 || time < cTime) cTime = time;

                        StripProfile->Fill(channel);
                        TimeProfile->Fill(time);
                        TDCProfile->Fill(channel,time);
                    }
                    if(cTime > SIGNAL_LOW && cTime < SIGNAL_HIGH){
                        cMultiplicity++;
                        ClusterSize->Fill(cSize);

                        hMultiplicity +=cSize;

                        cPosition /= (float)cSize;
                        ClusterPosition->Fill(cPosition);
                        ClusterTime->Fill(cTime);
                        ClusterProfile->Fill(cPosition,cTime);
                    }
                }
                HitMultiplicity->Fill(hMultiplicity);
                ClusterMultiplicity->Fill(cMultiplicity);
                if(cMultiplicity > 0) Efficiency->Fill(1);
                else Efficiency->Fill(0);
            }
        }
        float N = Efficiency->GetEntries();

        float eff = Efficiency->GetMean();
        float Deff = sqrt(eff*(1-eff)/N);

        float meancSize = ClusterSize->GetMean();
        float DmeancSize = 2*ClusterSize->GetRMS()/sqrt(N);

        ResultFile << eff << ',' << Deff << ',' << meancSize << ',' << DmeancSize <<'\n';

        StripProfile->Write();
        TimeProfile->Write();
        TDCProfile->Write();
        HitMultiplicity->Write();
        ClusterMultiplicity->Write();
        ClusterSize->Write();
        ClusterPosition->Write();
        ClusterTime->Write();
        ClusterProfile->Write();
        Efficiency->Write();
    } else {
        MSG_ERROR("Couldn't open the data file.\n");
        exit(EXIT_FAILURE);
    }

    //********************************************************************

    ResultFile.close();
    cout << "Creation of file " << ResultName << endl;

    ResultROOT.Close();
    cout << "Creation of file " << fNameROOT << endl;

    SortedFile.close();
}
