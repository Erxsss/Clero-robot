#include<bits/stdc++.h>
#include<vector>
#define ll long long
using namespace std;

int main(){
    ll n,w , max=0 ;
    cin>>n>>w;
    vector<ll> a(n) , b(n);
    for(ll i=0; i<n; i++){
        cin>>a[i]>>b[i];
    }
    int subsets=1<<n;
    for(ll s=0 ; s<subsets ; s++){
        ll ih=0,ezelhuun=0;
        for(ll k=0; k<n; k++){
            if(s & (1<<k)){
                ih+=a[k];
                ezelhuun+=b[k];
                // cout<<ih<<"ih"<<endl;
                // cout<<ezelhuun<<"ih"<<endl;
            }
        }
        if(ih>max && ezelhuun<=w){
            max=ih;
        }
    }
    cout<<max;
}
