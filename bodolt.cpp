#include<bits/stdc++.h>
#include<vector>
#define ll long long
using namespace std;

int main(){
    ll n,w;
    cin>>n>>w;
    vector<int> a(n);
    for(int i=0; i<n; i++){
        cin>>a[i];
    }
    int subsets=1<<n;
    for(int s=0 ; s<subsets ; s++){
        cout<<"{";
        for(int k=0; k<n; k++){
            if(s & (1<<k)){
                cout<<a[k]<<" ";
            }
        }
        cout<<"}"<<endl;
    }
    
}
