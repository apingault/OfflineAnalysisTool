// **********************************************************************
// *   Hit sorting functions
// *   Alexis Fagot
// *   2/3/2015
// *********************************************************************

#include "../include/SortDataFile.h"
#include "../include/MsgSvc.h"

using namespace std;

//*************************************************************************************

//TDCs give 2 informations :
//  - Channel (that leads to the strip)
//  - Time stamp
//This function allows to sort data pairs by increasing strip number.

//To sort the data, a quicksort algorithm is used. It consists in 3 steps :

//  1- Pick an element from the array A called Pivot (p) between its first (f) and
//last (l) element.
//  2- Reorder A so that all elements that all elements with values less than p come
//before p, while all elements with values greater than p come after it (equal values
//can go either way). After this partitionning, p is in its final position. This is
//called the Partition operation. In our case, this operation is released on the
//  3- Recursively apply the above steps to the sub-array of elements with smaller
//values and separately to the sub-array of elements with greater values.

//*************************************************************************************

int RandomPivot(int first,int last){        //Return a random element index in range
    return rand()%(last-first)+first;       //[first;last]
}

//*************************************************************************************

int Partition(vector< pair<int,int> >& A, int f, int l){
    pair<int,int> tPair;                    //Temporary pair to help swaping elements.

    int p = RandomPivot(f,l);               //Pick a random element as pivot and then
    tPair = A[p];                           //swap the pivot element with the last one.
    A[p] = A[l];
    A[l] = tPair;

    int j = f;                              //j starts at the first element of the array
                                            //or sub-array you want to sort. Every time
                                            //you find an element lower than the pivot,
                                            //you increment j.
                                            //At the end, j corresponds to the final of
                                            //the pivot.
    for(int i=f; i<l; i++){
        if(A[i].first < A[l].first){        //If element i is smaller than the pivot :
            tPair = A[i];                   //swap the element i with the element j
            A[i] = A[j];                    //which is the last element that was greater
            A[j] = tPair;                   //than the pivot.
            j++;                            //Increment j.
        }
    }

    tPair = A[j];                           //Finally swap element j which is greater
    A[j] = A[l];                            //than the pivot and the pivot. p is now
    A[l] = tPair;                           //at its final position j.
    return j;                               //Return the final position of the pivot.
}

//*************************************************************************************

void SortEvent(vector< pair<int,int> >& A, int f, int l){
    if(f < l){
        int pivot = Partition(A,f,l);       //Partition your array and get the pivot.

        if(A.size() > 2){
            if(pivot != f)
                SortEvent(A,f,pivot-1);     //Partition the lower sub-array.
            if(pivot != l)
                SortEvent(A,pivot+1,l);     //Partition the greater sub-array.
        }
    } else if (f > l){
        MSG_ERROR("Problem with the indexes : first index f > last index l\n");
        exit(EXIT_FAILURE);
    }
}

//*************************************************************************************
//This function sorts data by increasing strip number or increasing time stamp.
//Data format is :

//EventCount \t NumOfHits
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//EventCount \t NumOfHits
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//EventCount \t NumOfHits
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//EventCount \t NumOfHits
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//Channel \t TimeStamp
//...

//with a line of 0s in between each event (trigger).

void SortData(string fName){
    ifstream rawFile(fName.c_str(),ios::in);            //Open data file in read mode.

    if(rawFile){
        MSG_INFO("Open the file and start sorting.\n");
        vector < pair<int,int> > Data;                  //Array to contain hit list for
        Data.clear();                                   //each event.

        fName = "SORTED_" + fName;
        ofstream sortedFile(fName.c_str(), ios::out);   //Open output file in write mode.

        while(rawFile.good()){
            int nEvent = -1;
            int nHits = -1;

            rawFile >> nEvent >> nHits;                 //From data file, read headerline

            if(nEvent == -1 && nHits == -1){            //If nothing is read, this is the
                MSG_INFO("End of the file.\n");           //end.
                break;
            } else {
                for(int h=0; h<nHits; h++){             //else loop on every hit.
                    int strip = -1;
                    int time = -1;

                    rawFile >> strip >> time;           //Save data pairs into the array.
                    Data.push_back(make_pair(strip,time));
                }

                if(Data.size() > 0)                     //Sort the array
                    SortEvent(Data,0,Data.size()-1);
                                                        //Print the sorted data
                sortedFile << nEvent << " " << Data.size() << endl;
                for(int h = 0; h < Data.size(); h++)
                    sortedFile << Data[h].first << " " << Data[h].second << endl;
                Data.clear();
            }
        }
        sortedFile.close();
    } else {
        MSG_ERROR("Couldn't open data file to sort.\n");
        exit(EXIT_FAILURE);
    }
    rawFile.close();
}
