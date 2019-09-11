#include "capplacer.h"
#include <iomanip>
#include <nlohmann/json.hpp>
#include <cassert>

using namespace std;
using namespace nlohmann;

// These are in PnRDB
extern unsigned short JSON_Presentation (int font, int vp, int hp);
extern unsigned short JSON_STrans (bool reflect, bool abs_angle, bool abs_mag);
extern json JSON_TimeTime ();

Placer_Router_Cap::Placer_Router_Cap(const string& opath, const string& fpath, PnRDB::hierNode & current_node,
				     PnRDB::Drc_info &drc_info,
				     const map<string, PnRDB::lefMacro> &lefData,
				     bool dummy_flag, bool aspect_ratio, int num_aspect){
  cout<<"Enter"<<endl;
  Common_centroid_capacitor_aspect_ratio(opath, fpath, current_node, drc_info, lefData, dummy_flag, aspect_ratio, num_aspect);
  cout<<"Out"<<endl;
}

void Placer_Router_Cap::Placer_Router_Cap_clean(){

  std::cout<<"Enter clean 1"<<std::endl;
  std::cout<<"Enter clean 2"<<std::endl;
  CheckOutBlock = PnRDB::block();

  std::cout<<"Enter clean 3"<<std::endl;
  metal_width.clear();

  std::cout<<"Enter clean 4"<<std::endl;
  metal_direct.clear();

  std::cout<<"Enter clean 5"<<std::endl;
  metal_distance_ss.clear();

  std::cout<<"Enter clean 6"<<std::endl;
  via_width_x.clear();

  std::cout<<"Enter clean 7"<<std::endl;
  via_width_y.clear();

  std::cout<<"Enter clean 8"<<std::endl;
  via_cover_l.clear();

  std::cout<<"Enter clean 9"<<std::endl;
  via_cover_u.clear();

  std::cout<<"Enter clean 10"<<std::endl;
  Caps.clear();

  std::cout<<"Enter clean 11"<<std::endl;
  cap_pair_sequence.clear();

  std::cout<<"Enter clean 12"<<std::endl;
  net_sequence.clear();

  std::cout<<"Enter clean 13"<<std::endl;
  num_router_net_v.clear();

  std::cout<<"Enter clean 14"<<std::endl;
  num_router_net_h.clear();

  std::cout<<"Enter clean 15"<<std::endl;

  std::cout<<"Enter clean 16"<<std::endl;

  Nets_pos.clear();
  std::cout<<"Enter clean 17"<<std::endl;
  Nets_neg.clear();
}




void Placer_Router_Cap::Placer_Router_Cap_function(vector<int> & ki, vector<pair<string, string> > &cap_pin, const string& fpath, const string& unit_capacitor, const string& final_gds, bool cap_ratio, int cap_r, int cap_s, const PnRDB::Drc_info& drc_info, const map<string, PnRDB::lefMacro>& lefData, bool dummy_flag, const string& opath){

//dummy_flag is 1, dummy capacitor is added; Else, dummy capacitor do not exist.
//not added, needed to be added 

//initial DRC router

  //from lef file readin cap demension
  
  cout<<"step1"<<endl;
  string H_metal;
  int H_metal_index=-1;
  string V_metal;
  int V_metal_index=-1;

  string HV_via_metal;
  int HV_via_metal_index=-1;

  vector<string> obs;

  const auto &uc = lefData.at(unit_capacitor);

  for(unsigned int i=0;i<uc.interMetals.size();i++){
       int found = 0;
       for(unsigned int j=0;j<obs.size();j++){
          if(obs[j]==uc.interMetals[i].metal){
             found = 1;
            }
         }
       if(found == 0){obs.push_back(uc.interMetals[i].metal);}
      }

  unit_cap_demension.first = uc.width;
  unit_cap_demension.second= uc.height;
  int pin_minx = INT_MAX;
  int pin_miny = INT_MAX;
  string pin_metal;

  /*
   * SMB: This does something weird
   * it updates the LL if both the x and y coords are less than the previous best
   * So not necessarily the smallest x or the smalles t y
   */
  cout << "Find pin_minx, pin_miny" << endl;
  for(unsigned int i=0;i<uc.macroPins.size();i++){
      for(unsigned int j=0;j<uc.macroPins[i].pinContacts.size();j++){
	  const auto& pc = uc.macroPins[i].pinContacts[j];
	  const auto& r = pc.originBox.LL;
	  cout << "Cand " << r.x << " " << r.y;
	  if(r.x<=pin_minx and r.y<=pin_miny){
	      cout << " (Update)";
	      pin_minx = r.x;
	      pin_miny = r.y;
	      pin_metal = pc.metal;
	  }
	  cout << endl;
      }
  }
  cout << "Found pin_minx " << pin_minx << " pin_miny " << pin_miny << endl;
	  
   //determine which three layer are used for routing metal
	  
  const auto& mm = drc_info.Metalmap.at(pin_metal);

  if(drc_info.Metal_info.at(mm).direct == 1){ // metal pin is H
      H_metal = pin_metal;
      H_metal_index = mm;
      if(mm>0){ // metal pin has metal - 1 and
	  V_metal = drc_info.Metal_info[mm-1].name;
	  V_metal_index = mm-1;
      }else{
	  V_metal = drc_info.Metal_info[mm+1].name;
	  V_metal_index = mm+1;
      }
  }else{
      V_metal = pin_metal;
      V_metal_index = mm;
      if(mm>0){ // metal pin has metal - 1 and
	  H_metal = drc_info.Metal_info.at(mm-1).name;
	  H_metal_index = mm-1;
      }else{
	  H_metal = drc_info.Metal_info.at(mm+1).name;
	  H_metal_index = mm+1;
      }
  }
	  
  if(H_metal_index>V_metal_index){
      HV_via_metal = V_metal;
      HV_via_metal_index = V_metal_index;
  }else{
      HV_via_metal = H_metal;
      HV_via_metal_index = H_metal_index;
  }

  const auto& mv = drc_info.Metalmap.at(HV_via_metal);
  const auto& mvm = drc_info.Via_model.at(mv);
  if(mvm.LowerIdx==mm){
      auto& r = mvm.LowerRect[0];
      cout<<"rec x "<<r.x<<" rec y "<<r.y<<endl;
      shifting_x = pin_minx-r.x;
      shifting_y = pin_miny-r.y;
  }else if(mvm.UpperIdx==mm){
      auto& r = mvm.UpperRect[0];
      cout<<"rec x "<<r.x<<" rec y "<<r.y<<endl;
      shifting_x = pin_minx-r.x;
      shifting_y = pin_miny-r.y;
  }

  cout << "pin_minx " <<pin_minx << " "
       << "pin_miny " <<pin_miny << " "
       << "shifting_x "<<shifting_x<<" "
       << "shifting_y "<<shifting_y<<" "
       << "H_metal: " << H_metal << " "
       << "V_metal: " << V_metal << " "
       << "HV_via_metal: " << HV_via_metal << endl;

  cout<<"step2"<<endl;

  offset_x = 0;
  offset_y = 0;
  
  for(unsigned int i=0;i<drc_info.Metal_info.size();i++){
      metal_width.push_back(drc_info.Metal_info.at(i).width); //change 
      metal_width[i] = metal_width[i] / 2;
      metal_distance_ss.push_back(drc_info.Metal_info.at(i).dist_ss); //change //72
      metal_distance_ss[i] = metal_distance_ss[i]/2;
      metal_direct.push_back(drc_info.Metal_info.at(i).direct);
  }

  cout<<"step2.1"<<endl;

  min_dis_x = drc_info.Metal_info.at(V_metal_index).width
            + drc_info.Metal_info.at(V_metal_index).dist_ss;

  min_dis_y = drc_info.Metal_info.at(H_metal_index).width
            + drc_info.Metal_info.at(H_metal_index).dist_ss;

  min_dis_x *= 2;
  min_dis_y *= 2;

  cout<<"step2.2"<<endl;
  span_distance.first = min_dis_x;
  span_distance.second = 3*min_dis_y; //m1 distance
  cout<<"span_distance:" << span_distance.first << "," << span_distance.second << endl;

//initial cap information
  int net_size = ki.size();
  double sum = 0;
  double r;
  double s;
  for(int i=0;i<net_size;i++){
      sum = sum + ki[i];
  }
  r = ceil(sqrt(sum));
  s = ceil(sum/r);

  if(cap_ratio==1){ //cap_ratio = 1, pass the ratio by user otherwise calculate it by the code
      r = cap_r;
      s = cap_s;
  }    

//for dummy caps
  if(dummy_flag){
      r += 2;
      s += 2;
  }

  cout<<"step2.3"<<endl;
  for(int i=0;i<(int) r;i++){
     for(int j=0;j<(int) s;j++){
         cap temp_cap;
         temp_cap.index_x=(double) i;
         temp_cap.index_y=(double) j;
         temp_cap.x=unit_cap_demension.first/2 +  i* (unit_cap_demension.first+span_distance.first);
         temp_cap.y=unit_cap_demension.second/2 +  j* (unit_cap_demension.second+span_distance.second);
         temp_cap.net_index = -1;
         temp_cap.access = 0;
         Caps.push_back(temp_cap);
       }
    }
  

  cout<<"step2.4"<<endl;
//initial cap_pair_sequence
  double Cx = (r-1)/2;
  double Cy = (s-1)/2;
  vector<double> dis;
  vector<int> index;
  double distance;
  for(unsigned int i=0;i<Caps.size();i++){
        distance = sqrt((Caps[i].index_x-Cx)*(Caps[i].index_x-Cx)+(Caps[i].index_y-Cy)*(Caps[i].index_y-Cy));
        dis.push_back(distance);
        index.push_back(i);
    }
  //sort the distance
  int temp_index;
  for(unsigned int i=0;i<dis.size();i++){
     for(unsigned int j=i+1;j<dis.size();j++){
        if(dis[index[i]]>dis[index[j]]){
           temp_index = index[i];
           index[i]=index[j];
           index[j]=temp_index;
          }
        }
     }
  cout<<"step2.5"<<endl;
  //generate the cap pair sequence

  if (index.size()==1) {
      pair<int,int> temp_pair;
      temp_pair.first = index[0];
      temp_pair.second = -1;
      cap_pair_sequence.push_back(temp_pair);
  } else {
    
      int start_index=0;
      if(dis[index[0]]<dis[index[1]]){
	  pair<int,int> temp_pair;
	  temp_pair.first = index[0];
	  temp_pair.second = -1;
	  cap_pair_sequence.push_back(temp_pair);
	  start_index = 1;
      }

      //inital the rest pair sequence based on counterclockwise
      for(unsigned int i=start_index;i<dis.size();i++){
	  for(unsigned int j=i+1;j<dis.size();j++){
	      if(dis[index[i]]!=dis[index[j]]){
		  break;
              }
	      if(Caps[index[i]].index_x+Caps[index[j]].index_x==2*Cx and Caps[index[i]].index_y+Caps[index[j]].index_y==2*Cy){
		  pair<int,int> temp_pair;
		  temp_pair.first  = min( index[i], index[j]);
		  temp_pair.second = max( index[i], index[j]);
		  cap_pair_sequence.push_back(temp_pair);
		  break;
	      }
	  }
      }
  }


  cout<<"step2.6"<<endl;  

  if(dummy_flag){
      vector<pair<int,int> > temp_cap_pair_sequence;
      for(unsigned int i=0;i<cap_pair_sequence.size();i++){
	  if(cap_pair_sequence[i].second!=-1){
	      if(Caps[cap_pair_sequence[i].first].index_x!=0   and
		 Caps[cap_pair_sequence[i].first].index_x!=r-1 and
		 Caps[cap_pair_sequence[i].first].index_y!=0   and
		 Caps[cap_pair_sequence[i].first].index_y!=s-1 and
		 Caps[cap_pair_sequence[i].second].index_x!=0  and
		 Caps[cap_pair_sequence[i].second].index_x!=r-1 and
		 Caps[cap_pair_sequence[i].second].index_y!=0 and
		 Caps[cap_pair_sequence[i].second].index_y!=s-1){
		  temp_cap_pair_sequence.push_back(cap_pair_sequence[i]);
	      }
	  }else{
	      if(Caps[cap_pair_sequence[i].first].index_x!=0 and
		 Caps[cap_pair_sequence[i].first].index_x!=r-1 and
		 Caps[cap_pair_sequence[i].first].index_y!=0 and
		 Caps[cap_pair_sequence[i].first].index_y!=s-1){
		  temp_cap_pair_sequence.push_back(cap_pair_sequence[i]);
	      }
	  }
      }

      int num_pair= cap_pair_sequence.size();
      for(int i=0;i<num_pair;i++){
	  cap_pair_sequence.pop_back();
      }
      cap_pair_sequence= temp_cap_pair_sequence; //remove dummy capacitors
  }

// to be continued here.
  cout<<"step2.7"<<endl;
  initial_net_pair_sequence(ki,cap_pin);
  cout<<"step2.8"<<endl;
  string outfile=final_gds+".plt";
  cout<<"step2.9"<<endl;
  Router_Cap(ki,cap_pin, dummy_flag, cap_ratio, cap_r, cap_s);
  cout<<"step3"<<endl;
  GetPhysicalInfo_router( H_metal, H_metal_index, V_metal, V_metal_index, drc_info);
  cout<<"step4"<<endl;
  cal_offset(drc_info, H_metal_index, V_metal_index, HV_via_metal_index);
  cout<<"step5"<<endl;
  ExtractData(fpath ,unit_capacitor, final_gds, obs, drc_info, H_metal_index, V_metal_index, HV_via_metal_index, opath);
  cout<<"step6"<<endl;
  WriteJSON (fpath ,unit_capacitor, final_gds, drc_info, opath);
  cout<<"step7"<<endl;
  //PrintPlacer_Router_Cap(outfile);
  cout<<"step8"<<endl;


}


// DAK: General methods needed for layer mapping:  we should be using
// stoi(PnRDatabase::DRC_info.MaskID_Metal[layer])
static int
getLayerMask (const std::string & layer, const PnRDB::Drc_info & drc_info) {
    // DAK: These should be defined in a method that can load this map from a file / PDK
    int index = drc_info.Metalmap.at(layer);
    int mask = stoi(drc_info.MaskID_Metal.at(index));
    return mask;
}

static int
getLayerViaMask (const std::string & layer, const PnRDB::Drc_info & drc_info) {
    // DAK: These should be defined in a method that can load this map from a file / PDK
    int index = drc_info.Metalmap.at(layer);
    int mask = stoi(drc_info.MaskID_Via.at(index));
    return mask;
}

// DAK: Fills a contact with a 4 point rectangle
void
fillContact (PnRDB::contact& con, int* x, int*y) {
    for (int i = 0; i < 4; i++) {
	PnRDB::point temp_point;
	temp_point.x = x[i];
	temp_point.y = y[i];
	switch (i) {
	case 0: con.originBox.LL = temp_point; break;
	case 1: break;
	case 2: con.originBox.UR = temp_point; break;
	case 3: break;
	}
    }
    con.originCenter.x = (x[0]+x[2])/2;
    con.originCenter.y = (y[0]+y[2])/2;
}

class MinMax {
    int Min_x, Min_y, Max_x, Max_y;
public:
    MinMax() : Min_x(INT_MAX), Min_y(INT_MAX), Max_x(INT_MIN), Max_y(INT_MIN) {}
    void update( int x[], int y[]) {
	Min_x = min( x[0], Min_x);
	Max_x = max( x[2], Max_x);
	Min_y = min( y[0], Min_y);
	Max_y = max( y[2], Max_y);
    }
    int get_Min_x() const { return Min_x; }
    int get_Min_y() const { return Min_y; }
    int get_Max_x() const { return Max_x; }
    int get_Max_y() const { return Max_y; }
};

void
Placer_Router_Cap::ExtractData (const string& fpath, const string& unit_capacitor, const string& final_gds, vector<string> & obs, const PnRDB::Drc_info & drc_info, int H_metal, int V_metal, int HV_via_index, const string& opath) {
    string topGDS_loc = opath+final_gds+".gds";
    int gds_unit = 20;
    //writing metals
    int x[5], y[5];
  
//    int width = metal_width[0];
    MinMax minmax;
    //for positive nets
    cout<<"Extract Data Step 1"<<endl;
    for (unsigned int i = 0; i < Nets_pos.size(); i++) {//for each net
	PnRDB::pin temp_Pins;
	for (unsigned int j = 0; j < Nets_pos[i].start_conection_coord.size(); j++) { //for segment

            int width = drc_info.Metal_info.at(drc_info.Metalmap.at(Nets_pos[i].metal[j])).width/2;

	    fillPathBoundingBox (x, y, Nets_pos[i].start_conection_coord[j],
				 Nets_pos[i].end_conection_coord[j], width);

	    minmax.update( x, y);

	    PnRDB::contact temp_contact;
            fillContact (temp_contact, x, y);

	    for (int i = 0; i < 5; i++) {
		x[i] *= gds_unit;
		y[i] *= gds_unit;
	    }
	    temp_contact.metal = Nets_pos[i].metal[j];
	    if (Nets_pos[i].Is_pin[j] == 1) {
		temp_Pins.name = Nets_pos[i].name;
		temp_Pins.pinContacts.push_back(temp_contact);
	    }
	    CheckOutBlock.interMetals.push_back(temp_contact);
	}   
	CheckOutBlock.blockPins.push_back(temp_Pins);
    }
    cout<<"Extract Data Step 2"<<endl;
    //for neg nets
    for (unsigned int i = 0; i < Nets_neg.size(); i++) {//for each net
	PnRDB::pin temp_Pins_neg;
	for (unsigned int j = 0; j < Nets_neg[i].start_conection_coord.size(); j++) { //for segment

            int width = drc_info.Metal_info.at(drc_info.Metalmap.at(Nets_neg[i].metal[j])).width/2;

	    fillPathBoundingBox (x, y, Nets_neg[i].start_conection_coord[j],
				 Nets_neg[i].end_conection_coord[j], width);
            
	    minmax.update( x, y);

            PnRDB::contact temp_contact;
	    fillContact (temp_contact, x, y);

	    for (int i = 0; i < 5; i++) {
		x[i] *= gds_unit;
		y[i] *= gds_unit;
	    }
            temp_contact.metal = Nets_neg[i].metal[j];
	    if (Nets_neg[i].Is_pin[j] == 1) {
                temp_Pins_neg.name = Nets_neg[i].name;
                temp_Pins_neg.pinContacts.push_back(temp_contact);
	    }
	    CheckOutBlock.interMetals.push_back(temp_contact);
	}
	CheckOutBlock.blockPins.push_back(temp_Pins_neg);
    }
    cout<<"Extract Data Step 3"<<endl;
    //wirting vias
    //for positive net
    //width = via_width[0];
    for (unsigned int i = 0; i < Nets_pos.size(); i++) {
	for (unsigned int j = 0; j < Nets_pos[i].via.size(); j++) {//the size of via needs to be modified according to different PDK
            cout<<"Extract Data Step 3.1"<<endl;
            int width = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_pos[i].via_metal[j])).ViaRect[1].x;

 	    x[0]=Nets_pos[i].via[j].first - width+offset_x;
	    x[1]=Nets_pos[i].via[j].first - width+offset_x;
	    x[2]=Nets_pos[i].via[j].first + width+offset_x;
	    x[3]=Nets_pos[i].via[j].first + width+offset_x;
	    x[4]=x[0];

            width = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_pos[i].via_metal[j])).ViaRect[1].y;
        
	    y[0]=Nets_pos[i].via[j].second - width+offset_y;
	    y[1]=Nets_pos[i].via[j].second + width+offset_y;
	    y[2]=Nets_pos[i].via[j].second + width+offset_y;
	    y[3]=Nets_pos[i].via[j].second - width+offset_y;
	    y[4]=y[0];
        
	    minmax.update( x, y);

	    PnRDB::contact temp_contact;
	    fillContact (temp_contact, x, y);
	    for (int i = 0; i < 5; i++) {
		x[i] *= gds_unit;
		y[i] *= gds_unit;
	    }

//this part needs modify 2019/6/3
            cout<<"Extract Data Step 3.2"<<endl;
	    PnRDB::Via temp_via;
	    PnRDB::contact upper_contact;
	    PnRDB::contact lower_contact;
	    upper_contact.placedCenter = temp_contact.placedCenter;
	    lower_contact.placedCenter = temp_contact.placedCenter;
            cout<<"Extract Data Step 3.3"<<endl;
	    PnRDB::contact h_contact;
            int via_model_index;
            via_model_index = drc_info.Metalmap.at(Nets_pos[i].via_metal[j]);
            temp_contact.metal = drc_info.Via_model.at(via_model_index).name;
            h_contact.originBox.LL = drc_info.Via_model.at(via_model_index).UpperRect[0];
            h_contact.originBox.UR = drc_info.Via_model.at(via_model_index).UpperRect[1];
            //cout<<"Extract Data Step 3.31"<<endl;
            h_contact.originBox.LL.x += temp_contact.placedCenter.x;
            h_contact.originBox.LL.y += temp_contact.placedCenter.y;

            h_contact.originBox.UR.x += temp_contact.placedCenter.x;
            h_contact.originBox.UR.y += temp_contact.placedCenter.y;
            cout<<"Extract Data Step 3.4"<<endl;
	    PnRDB::contact v_contact;
            v_contact.originBox.LL = drc_info.Via_model.at(via_model_index).LowerRect[0];
            v_contact.originBox.UR = drc_info.Via_model.at(via_model_index).LowerRect[1];

            v_contact.originBox.LL.x += temp_contact.placedCenter.x;
            v_contact.originBox.LL.y += temp_contact.placedCenter.y;

            v_contact.originBox.UR.x += temp_contact.placedCenter.x;
            v_contact.originBox.UR.y += temp_contact.placedCenter.y;

            cout<<"Extract Data Step 3.5"<<endl;
            lower_contact.metal = drc_info.Metal_info.at(drc_info.Via_model.at(via_model_index).LowerIdx).name;
            upper_contact.metal = drc_info.Metal_info.at(drc_info.Via_model.at(via_model_index).UpperIdx).name;
            lower_contact.originBox = v_contact.originBox;
            upper_contact.originBox = h_contact.originBox;
            temp_via.model_index = via_model_index;
            cout<<"Extract Data Step 3.6"<<endl;
	    temp_via.placedpos = temp_contact.originCenter;
	    temp_via.ViaRect = temp_contact;
	    temp_via.LowerMetalRect = lower_contact;
	    temp_via.UpperMetalRect = upper_contact;
	    CheckOutBlock.interVias.push_back(temp_via);
	}
    }
    cout<<"Extract Data Step 4"<<endl;
    //for negative net
    for (unsigned int i = 0; i < Nets_neg.size(); i++) {
	for (unsigned int j = 0; j <Nets_neg[i].via.size(); j++) {//the size of via needs to be modified according to different PDK
            cout<<"Extract Data Step 4.1"<<endl;
            int width = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_neg[i].via_metal[j])).ViaRect[1].x;

	    x[0]=Nets_neg[i].via[j].first - width+offset_x;
	    x[1]=Nets_neg[i].via[j].first - width+offset_x;
	    x[2]=Nets_neg[i].via[j].first + width+offset_x;
	    x[3]=Nets_neg[i].via[j].first + width+offset_x;
	    x[4]=x[0];

            width = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_neg[i].via_metal[j])).ViaRect[1].y;        
	    y[0]=Nets_neg[i].via[j].second - width+offset_y;
	    y[1]=Nets_neg[i].via[j].second + width+offset_y;
	    y[2]=Nets_neg[i].via[j].second + width+offset_y;
	    y[3]=Nets_neg[i].via[j].second - width+offset_y;
	    y[4]=y[0];
        
	    minmax.update( x, y);

	    PnRDB::contact temp_contact;
	    fillContact (temp_contact, x, y);

	    for (int i = 0; i < 5; i++) {
		x[i] *= gds_unit;
		y[i] *= gds_unit;
	    }
            cout<<"Extract Data Step 4.2"<<endl;
	    PnRDB::Via temp_via;
	    PnRDB::contact upper_contact;
	    PnRDB::contact lower_contact;
	    upper_contact.placedCenter = temp_contact.placedCenter;
	    lower_contact.placedCenter = temp_contact.placedCenter;

//this part needs to be modify


	    PnRDB::contact h_contact;
            int via_model_index;
            via_model_index = drc_info.Metalmap.at(Nets_neg[i].via_metal[j]);
            temp_contact.metal = drc_info.Via_model.at(via_model_index).name;
            h_contact.originBox.LL = drc_info.Via_model.at(via_model_index).UpperRect[0];
            h_contact.originBox.UR = drc_info.Via_model.at(via_model_index).UpperRect[1];

            h_contact.originBox.LL.x += temp_contact.placedCenter.x;
            h_contact.originBox.LL.y += temp_contact.placedCenter.y;

            h_contact.originBox.UR.x += temp_contact.placedCenter.x;
            h_contact.originBox.UR.y += temp_contact.placedCenter.y;

            cout<<"Extract Data Step 4.25"<<endl;
	    PnRDB::contact v_contact;
            v_contact.originBox.LL = drc_info.Via_model.at(via_model_index).LowerRect[0];
            v_contact.originBox.UR = drc_info.Via_model.at(via_model_index).LowerRect[1];

            v_contact.originBox.LL.x += temp_contact.placedCenter.x;
            v_contact.originBox.LL.y += temp_contact.placedCenter.y;

            v_contact.originBox.UR.x += temp_contact.placedCenter.x;
            v_contact.originBox.UR.y += temp_contact.placedCenter.y;

            cout<<"Extract Data Step 4.3"<<endl;
            lower_contact.metal = drc_info.Metal_info.at(drc_info.Via_model.at(via_model_index).LowerIdx).name;
            upper_contact.metal = drc_info.Metal_info.at(drc_info.Via_model.at(via_model_index).UpperIdx).name;
            lower_contact.originBox = v_contact.originBox;
            upper_contact.originBox = h_contact.originBox;
            temp_via.model_index = via_model_index;
            cout<<"Extract Data Step 4.4"<<endl;
	    temp_via.placedpos = temp_contact.originCenter;
	    temp_via.ViaRect = temp_contact;
	    temp_via.LowerMetalRect = lower_contact;
	    temp_via.UpperMetalRect = upper_contact;
	    CheckOutBlock.interVias.push_back(temp_via);
	}
    }
    CheckOutBlock.orient = PnRDB::Omark(0); //need modify
    cout<<"Extract Data Step 5"<<endl;
    for (unsigned int i = 0; i < Caps.size(); i++) {
	x[0]=Caps[i].x - unit_cap_demension.first/2+offset_x;
	x[1]=Caps[i].x - unit_cap_demension.first/2+offset_x;
	x[2]=Caps[i].x + unit_cap_demension.first/2+offset_x;
	x[3]=Caps[i].x + unit_cap_demension.first/2+offset_x;
	x[4]=x[0];
       
	y[0]=Caps[i].y - unit_cap_demension.second/2+offset_y;
	y[1]=Caps[i].y + unit_cap_demension.second/2+offset_y;
	y[2]=Caps[i].y + unit_cap_demension.second/2+offset_y;
	y[3]=Caps[i].y - unit_cap_demension.second/2+offset_y;
	y[4]=y[0];
     
	minmax.update( x, y);

//this part need modify, here the 
	PnRDB::contact temp_contact;
	fillContact (temp_contact, x, y);

        for(unsigned int i=0;i<obs.size();i++){
            temp_contact.metal = obs[i];
            CheckOutBlock.interMetals.push_back(temp_contact);
           }
    }
    cout<<"Extract Data Step 7"<<endl;


    int coverage_x;
    int coverage_y;
  
    const auto& vm = drc_info.Via_model.at(HV_via_index);

    if(drc_info.Via_model[HV_via_index].LowerIdx == V_metal){
	coverage_y = vm.ViaRect[0].y - vm.LowerRect[0].y;
	coverage_x = vm.ViaRect[0].x - vm.UpperRect[0].x;
    }else{
       coverage_y = vm.ViaRect[0].y - vm.UpperRect[0].y;
       coverage_x = vm.ViaRect[0].x - vm.LowerRect[0].x;
    }

    int Min_x = minmax.get_Min_x();
    int Min_y = minmax.get_Min_y();
    int Max_x = minmax.get_Max_x();
    int Max_y = minmax.get_Max_y();

    int deltax = drc_info.Metal_info.at(V_metal).grid_unit_x
               - drc_info.Metal_info.at(V_metal).width/2-coverage_x;
    Min_x -= deltax;
    Max_x += deltax;
	
    int deltay = drc_info.Metal_info.at(H_metal).grid_unit_y
	       - drc_info.Metal_info.at(H_metal).width/2-coverage_y;
    Min_y -= deltay;
    Max_y += deltay;

    const auto gu = drc_info.Metal_info[V_metal].grid_unit_x;
    Max_x = ceil(Max_x/gu)*gu;

    CheckOutBlock.gdsFile = topGDS_loc;
    PnRDB::point temp_point;
    temp_point.x = Min_x;
    temp_point.y = Min_y;
    CheckOutBlock.originBox.LL = temp_point;
    temp_point.x = Max_x;
    temp_point.y = Max_y;
    CheckOutBlock.originBox.UR = temp_point;
    CheckOutBlock.originCenter.x = (CheckOutBlock.originBox.LL.x + CheckOutBlock.originBox.UR.x)/2;
    CheckOutBlock.originCenter.y = (CheckOutBlock.originBox.LL.y + CheckOutBlock.originBox.UR.y)/2;
    CheckOutBlock.width = CheckOutBlock.originBox.UR.x-CheckOutBlock.originBox.LL.x;
    CheckOutBlock.height = CheckOutBlock.originBox.UR.y-CheckOutBlock.originBox.LL.y;
}

void
Placer_Router_Cap::cal_offset(const PnRDB::Drc_info &drc_info, int H_metal, int V_metal, int HV_via_index) {
    int x[5], y[5];
    //int width = metal_width[0];

    MinMax minmax;

    //for positive nets
    // vector<pair<double,double> f, s;  int width
  
    for (unsigned int i = 0; i< Nets_pos.size(); i++) {//for each net
	for (unsigned int j = 0; j< Nets_pos[i].start_conection_coord.size();j++) { //for segment
            int width = drc_info.Metal_info.at(drc_info.Metalmap.at(Nets_pos[i].metal[j])).width/2;
	    fillPathBoundingBox (x, y, Nets_pos[i].start_conection_coord[j],
				 Nets_pos[i].end_conection_coord[j], width);

	    minmax.update( x, y);
        }
    }
  
    //for neg nets
    for (unsigned int i = 0; i <  Nets_neg.size(); i++) {//for each net
	for (unsigned int j = 0; j <  Nets_neg[i].start_conection_coord.size();j++) { //for segment
            int width = drc_info.Metal_info.at(drc_info.Metalmap.at(Nets_neg[i].metal[j])).width/2;
	    fillPathBoundingBox (x, y, Nets_neg[i].start_conection_coord[j],
				 Nets_neg[i].end_conection_coord[j], width);

	    minmax.update( x, y);
        }
    }
  
    //wirting vias
    //for positive net
    //width  =  via_width[0];
    for (unsigned int i = 0; i < Nets_pos.size(); i++) {
	for (unsigned int j = 0; j < Nets_pos[i].via.size(); j++) {//the size of via needs to be modified according to different PDK

	    const auto& vm = Nets_pos[i].via_metal[j];
	    const auto& r = drc_info.Via_model.at(drc_info.Metalmap.at(vm)).ViaRect[1];
	    const auto& n = Nets_pos[i].via[j];

	    x[0] = n.first - r.x;
	    x[1] = n.first - r.x;
	    x[2] = n.first + r.x;
	    x[3] = n.first + r.x;
	    x[4] = x[0];

	    y[0] = n.second - r.y;
	    y[1] = n.second + r.y;
	    y[2] = n.second + r.y;
	    y[3] = n.second - r.y;
	    y[4] = y[0];

	    minmax.update( x, y);
	}
    }
    
    //for negative net
    for (unsigned int i = 0;i < Nets_neg.size(); i++) {
	for (unsigned int j = 0; j < Nets_neg[i].via.size(); j++) {//the size of via needs to be modified according to different PDK
 
            int width = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_neg[i].via_metal[j])).ViaRect[1].x;

	    x[0] = Nets_neg[i].via[j].first - width;
	    x[1] = Nets_neg[i].via[j].first - width;
	    x[2] = Nets_neg[i].via[j].first + width;
	    x[3] = Nets_neg[i].via[j].first + width;
	    x[4] = x[0];

            width = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_neg[i].via_metal[j])).ViaRect[1].y;
        
	    y[0] = Nets_neg[i].via[j].second - width;
	    y[1] = Nets_neg[i].via[j].second + width;
	    y[2] = Nets_neg[i].via[j].second + width;
	    y[3] = Nets_neg[i].via[j].second - width;
	    y[4] = y[0];
        
	    minmax.update( x, y);
	}
    }
  
    for (unsigned int i = 0; i < Caps.size(); i++) {
	x[0] = Caps[i].x - unit_cap_demension.first/2;
	x[1] = Caps[i].x - unit_cap_demension.first/2;
	x[2] = Caps[i].x + unit_cap_demension.first/2;
	x[3] = Caps[i].x + unit_cap_demension.first/2;
	x[4] = x[0];
       
	y[0] = Caps[i].y - unit_cap_demension.second/2;
	y[1] = Caps[i].y + unit_cap_demension.second/2;
	y[2] = Caps[i].y + unit_cap_demension.second/2;
	y[3] = Caps[i].y - unit_cap_demension.second/2;
	y[4] = y[0];

	minmax.update( x, y);
    }

    int coverage_x;
    int coverage_y;
  
    const auto& vm = drc_info.Via_model[HV_via_index]; 
    if(vm.LowerIdx == V_metal){
       coverage_y = vm.ViaRect[0].y - vm.LowerRect[0].y;
       coverage_x = vm.ViaRect[0].x - vm.UpperRect[0].x;
    }else{
       coverage_y = vm.ViaRect[0].y - vm.UpperRect[0].y;
       coverage_x = vm.ViaRect[0].x - vm.LowerRect[0].x;
    }

    const auto& vmv = drc_info.Metal_info[V_metal];
    offset_x = 0-minmax.get_Min_x();
    offset_x = offset_x + vmv.grid_unit_x - vmv.width/2 - coverage_x;

    const auto& vmh = drc_info.Metal_info[H_metal];
    offset_y = 0-minmax.get_Min_y();
    offset_y = offset_y + vmh.grid_unit_y - vmh.width/2 - coverage_y;
    
}

void Placer_Router_Cap::initial_net_pair_sequence(vector<int> & ki, vector<pair<string, string> > & cap_pin){
//initial net pair sequence
  cout<<"test case 1"<<endl;
  vector<pair<int,int> > S_unique;
  vector<pair<int,int> > S_unit_unit;
  vector<pair<int,int> > S_unit_odd;
  vector<pair<int,int> > S_odd_odd;
  vector<pair<int,int> > S;
  pair<int,int> temp_pair;
  vector<int> C_unit;
  vector<int> C_odd;
  vector<int> C_even;
  for(unsigned int i=0;i<ki.size();i++){
       if(ki[i]==1){
           C_unit.push_back(i);
         }else if(ki[i]%2==1){
           C_odd.push_back(i);
         }else{
           C_even.push_back(i);
         }
     }
  //initial net pair sequence for pair
  cout<<"test case 2"<<endl;
  int size;
  for(unsigned int i=0;i<C_even.size();i++){
     size = ki[C_even[i]];
     while(size>1){
         temp_pair.first=C_even[i];
         temp_pair.second=C_even[i];
         size=size-2;
         S.push_back(temp_pair);
        }
     }
  for(unsigned int i=0;i<C_odd.size();i++){
     size = ki[C_odd[i]];
     while(size>1){
         temp_pair.first=C_odd[i];
         temp_pair.second=C_odd[i];
         size=size-2;
         S.push_back(temp_pair);
        }
     }

  cout<<"test case 3"<<endl;
  //initial net pair sequence for odd 
  int num_unit = C_unit.size();
  int num_odd = C_odd.size();
  while(num_odd>1){
     temp_pair.first = C_odd[num_odd-1];
     temp_pair.second = C_odd[num_odd-2];
     C_odd.pop_back();
     C_odd.pop_back();
     num_odd = num_odd -2;
     S_odd_odd.push_back(temp_pair);
  }
  if(num_odd==1 and num_unit>0){
     temp_pair.first = C_odd[num_odd-1];
     temp_pair.second = C_unit[num_unit-1];
     C_unit.pop_back();
     num_unit = num_unit -1;
     num_odd = num_odd -1;
     S_unit_odd.push_back(temp_pair);
  }else if(num_odd==1 and num_unit ==0){
     temp_pair.first = C_odd[num_odd-1];
     temp_pair.second = -1;
     num_odd = num_odd -1;
     S_unique.push_back(temp_pair);
  }
  ////initial net pair sequence for unit
  while(num_unit>1){
     temp_pair.first = C_unit[num_unit-1];
     temp_pair.second = C_unit[num_unit-2];
     C_unit.pop_back();
     C_unit.pop_back();
     num_unit = num_unit -2;
     S_unit_unit.push_back(temp_pair);
  }
  if(num_unit==1){
    temp_pair.first = C_unit[num_unit-1];
     temp_pair.second = -1;
     num_unit = num_unit -1;
     S_unique.push_back(temp_pair);
  }
  if(S_unique.size()>1){
     std::cout<<"Error in S_unique"<<std::endl;
    }
  for(unsigned int i=0;i<S_unique.size();i++){
      net_sequence.push_back(S_unique[i]);
    }
  for(unsigned int i=0;i<S_unit_unit.size();i++){
      net_sequence.push_back(S_unit_unit[i]);
    }
  for(unsigned int i=0;i<S_unit_odd.size();i++){
      net_sequence.push_back(S_unit_odd[i]);
    }
  for(unsigned int i=0;i<S_odd_odd.size();i++){
      net_sequence.push_back(S_odd_odd[i]);
    }
  for(unsigned int i=0;i<S.size();i++){
      net_sequence.push_back(S[i]);
    }
  cout<<"test case 4"<<endl;
  net temp_net;

  for(unsigned int i=0;i<ki.size()+1;i++){
     if(i<ki.size()){
       temp_net.name = cap_pin[i].second;
     }else{
       temp_net.name = "dummy_gnd";
     }
     Nets_pos.push_back(temp_net);
   }

  cout<<"test case 4.5"<<endl;
  int start_index=0;
  for(unsigned int i=0;i<net_sequence.size();i++){
     if(net_sequence[i].second==-1){
        cout<<"test case 4.51"<<endl;
        cout<<net_sequence[i].first<<endl;
        cout<<cap_pair_sequence[start_index].first<<endl;
        Nets_pos[net_sequence[i].first].cap_index.push_back(cap_pair_sequence[start_index].first);
        cout<<"1"<<endl;
        Caps[cap_pair_sequence[start_index].first].net_index = net_sequence[i].first;
        cout<<"2"<<endl;
        start_index = start_index+1;
        cout<<"test case 4.52"<<endl;
       }else if(net_sequence[i].second!=-1 and cap_pair_sequence[start_index].second == -1){
        cout<<"test case 4.53"<<endl;
        start_index = start_index+1;
        Nets_pos[net_sequence[i].first].cap_index.push_back(cap_pair_sequence[start_index].first);
        cout<<"1"<<endl;
        Nets_pos[net_sequence[i].second].cap_index.push_back(cap_pair_sequence[start_index].second);
        cout<<"2"<<endl;
        Caps[cap_pair_sequence[start_index].first].net_index = net_sequence[i].first;
        cout<<"3"<<endl;
        Caps[cap_pair_sequence[start_index].second].net_index = net_sequence[i].second;
        start_index = start_index+1;
        cout<<"test case 4.54"<<endl;
       }else if(net_sequence[i].second!=-1 and cap_pair_sequence[start_index].second != -1){
        cout<<"test case 4.55"<<endl;
        Nets_pos[net_sequence[i].first].cap_index.push_back(cap_pair_sequence[start_index].first);
        cout<<"1"<<endl;
        Nets_pos[net_sequence[i].second].cap_index.push_back(cap_pair_sequence[start_index].second);
        cout<<"2"<<endl;
        Caps[cap_pair_sequence[start_index].first].net_index = net_sequence[i].first;
        cout<<"3"<<endl;
        Caps[cap_pair_sequence[start_index].second].net_index = net_sequence[i].second;
        start_index = start_index+1;
        cout<<"test case 4.56"<<endl;
      }
     }
  //add a net for dummy capacitor


  // a dummy net is added for dummy capacitor
  cout<<"test case 5"<<endl;

  int dummy_cap = Nets_pos.size();
  for(unsigned int i=0;i<Caps.size();i++){
      if(Caps[i].net_index==-1){
         Nets_pos[dummy_cap-1].cap_index.push_back(i);
        }
     }
}


void Placer_Router_Cap::perturbation_pair_sequence(){
//perturbate pair sequence

}

void Placer_Router_Cap::Placer_Cap(vector<int> & ki){
  
}

void Placer_Router_Cap::Router_Cap(vector<int> & ki, vector<pair<string, string> > &cap_pin, bool dummy_flag, bool cap_ratio, int cap_r, int cap_s){

  cout<<"broken down 1"<<endl;
//route for cap
  for(unsigned int i=0;i<Nets_pos.size();i++){ // for each net
     for(unsigned int j=0;j<Nets_pos[i].cap_index.size();j++){ //for each unaccessed cap
        if(Caps[Nets_pos[i].cap_index[j]].access==0){
           connection_set temp_set;
           temp_set.cap_index.push_back(Nets_pos[i].cap_index[j]); //new set & marked accessed
           Caps[Nets_pos[i].cap_index[j]].access = 1;
           //found its neighbor recursively
           found_neighbor(j,Nets_pos[i],temp_set);
           Nets_pos[i].Set.push_back(temp_set);
          }
        } 
    }
  cout<<"broken down 2"<<endl;
  int net_size = ki.size();
  double sum = 0;
  double r;
  double s;
  for(int i=0;i<net_size;i++){
     sum = sum + ki[i];
    }
  cout<<"broken down 3"<<endl;

   r = ceil(sqrt(sum));
   s = ceil(sum/r);

   if(cap_ratio){
   r = cap_r;
   s = cap_s;
   }

   if(dummy_flag){
   r= r+2;
   s= s+2;
   }

   double Cx = (r)/2; //note this is different
   double Cy = (s)/2; //note this is different
//create router line for each net (cap) vertical 

  cout<<"broken down 3.1"<<endl;
  for(unsigned int i=0;i<Nets_pos.size();i++){
     for(unsigned int j=0;j<Nets_pos[i].Set.size();j++){
         cout<<"broken down 3.11"<<endl;
         connection_set temp_router_line;
         //initial temp_router_line
         for(int k=0;k<=r;k++){
             temp_router_line.cap_index.push_back(0);
            }
         cout<<"broken down 3.2"<<endl;
         for(unsigned int l=0;l<Nets_pos[i].Set[j].cap_index.size();l++){
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_x]=1;
             cout<<"broken down 3.3"<<endl;
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_x+1]=1;
             cout<<"broken down 3.4"<<endl;
             temp_router_line.cap_index[2*Cx-Caps[Nets_pos[i].Set[j].cap_index[l]].index_x]=1;
             cout<<"broken down 3.5"<<endl;
             temp_router_line.cap_index[2*Cx-Caps[Nets_pos[i].Set[j].cap_index[l]].index_x-1]=1;//-1
            }
         cout<<"broken down 3.6"<<endl;
         Nets_pos[i].router_line_v.push_back(temp_router_line);
         cout<<"broken down 3.7"<<endl;
        }
     }

  cout<<"broken down 4"<<endl;
//common overlap checking vertical
  for(unsigned int i=0;i<Nets_pos.size();i++){
     for(int j=0;j<=r;j++){
          Nets_pos[i].routable_line_v.push_back(1);
        }
     for(unsigned int k=0;k<Nets_pos[i].router_line_v.size();k++){
          for(unsigned int l=0;l<Nets_pos[i].router_line_v[k].cap_index.size();l++){
             Nets_pos[i].routable_line_v[l] =(int) Nets_pos[i].routable_line_v[l] and Nets_pos[i].router_line_v[k].cap_index[l];
             }
        }
     }

  cout<<"broken down 5"<<endl;
//create router line for each net (cap) horizontal
  for(unsigned int i=0;i<Nets_pos.size();i++){
     for(unsigned int j=0;j<Nets_pos[i].Set.size();j++){
         connection_set temp_router_line;
         //initial temp_router_line
         for(int k=0;k<=s;k++){
             temp_router_line.cap_index.push_back(0);
            }
         for(unsigned int l=0;l<Nets_pos[i].Set[j].cap_index.size();l++){
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_y]=1;
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_y+1]=1;
             temp_router_line.cap_index[2*Cy-Caps[Nets_pos[i].Set[j].cap_index[l]].index_y]=1;
             temp_router_line.cap_index[2*Cy-Caps[Nets_pos[i].Set[j].cap_index[l]].index_y-1]=1;//-1
            }
         Nets_pos[i].router_line_h.push_back(temp_router_line);
        }
     }

  cout<<"broken down 6"<<endl;
//common overlap checking horizontal
  for(unsigned int i=0;i<Nets_pos.size();i++){
     for(int j=0;j<=s;j++){
          Nets_pos[i].routable_line_h.push_back(1);
        }
     for(unsigned int k=0;k<Nets_pos[i].router_line_h.size();k++){
          for(unsigned int l=0;l<Nets_pos[i].router_line_h[k].cap_index.size();l++){
             Nets_pos[i].routable_line_h[l] =(int) Nets_pos[i].routable_line_h[l] and Nets_pos[i].router_line_h[k].cap_index[l];
             }
        }
     }


  cout<<"broken down 7"<<endl;
//create router line for each net (cap) half vertical 
  for(unsigned int i=0;i<Nets_pos.size();i++){
     for(unsigned int j=0;j<Nets_pos[i].Set.size();j++){
         connection_set temp_router_line;
         //initial temp_router_line
         for(int k=0;k<=r;k++){
             temp_router_line.cap_index.push_back(0);
            }
         for(unsigned int l=0;l<Nets_pos[i].Set[j].cap_index.size();l++){
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_x]=1;
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_x+1]=1;
            }
         Nets_pos[i].half_router_line_v.push_back(temp_router_line);
        }
     }

  cout<<"broken down 8"<<endl;
//create router line for each net (cap) half horizontal
  for(unsigned int i=0;i<Nets_pos.size();i++){
     for(unsigned int j=0;j<Nets_pos[i].Set.size();j++){
         connection_set temp_router_line;
         //initial temp_router_line
         for(int k=0;k<=s;k++){
             temp_router_line.cap_index.push_back(0);
            }
         for(unsigned int l=0;l<Nets_pos[i].Set[j].cap_index.size();l++){
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_y]=1;
             temp_router_line.cap_index[Caps[Nets_pos[i].Set[j].cap_index[l]].index_y+1]=1;
            }
         Nets_pos[i].half_router_line_h.push_back(temp_router_line);
        }
     }
  

  cout<<"broken down 9"<<endl;
  //initialize num_router_net_v and num_router_net_h
  for(int i=0;i<=r;i++){num_router_net_v.push_back(0);}
  for(int i=0;i<=s;i++){num_router_net_h.push_back(0);}
  
  Nets_neg = Nets_pos;
  for(unsigned int i=0;i<Nets_pos.size();i++){
       if(i!=Nets_pos.size()-1){
           Nets_neg[i].name = cap_pin[i].first;
         }else{
           Nets_neg[i].name = "dummy_gnd";
         }
     }
  
  cout<<"broken down 10"<<endl;
//Next work for good router
//sample route methodology just for v pos
  for(unsigned int i=0;i<Nets_pos.size();i++){
      for(int k=0;k<=r;k++){Nets_pos[i].line_v.push_back(0);}
      int sum=0;
      for(unsigned int k=0;k<Nets_pos[i].routable_line_v.size();k++){sum=sum+Nets_pos[i].routable_line_v[k];}
      if(sum>0){
         //use the information of routable_line_v
         int router_num=Nets_pos.size();
         int choosed_router=-1;
         for(int j=0;j<=Cx;j++){
              if(Nets_pos[i].routable_line_v[j]==1){
                  if(num_router_net_v[j]<=router_num){
                     choosed_router=j;
                     router_num = num_router_net_v[j];
                    }
                }
            }
         if(1){
         Nets_pos[i].line_v[choosed_router]=1;
         Nets_pos[i].line_v[2*Cx-choosed_router]=1;
         num_router_net_v[choosed_router]=num_router_net_v[choosed_router]+1;
         num_router_net_v[2*Cx-choosed_router]=num_router_net_v[2*Cx-choosed_router]+1;
            }else{
            Nets_pos[i].line_v[choosed_router]=1;
            num_router_net_v[choosed_router]=num_router_net_v[choosed_router]+1;
            }
             
       }else{
         //use the information of half_routable_line_v
         for(unsigned int l=0;l<Nets_pos[i].half_router_line_v.size();l++){
             int found=0;
             for(unsigned int k=0;k<Nets_pos[i].half_router_line_v[l].cap_index.size();k++){
                 if(Nets_pos[i].half_router_line_v[l].cap_index[k]==1 and Nets_pos[i].line_v[k]==1){
                   found =1;
                   }
                }
             if(found ==0){
                int router_num=Nets_pos.size();
                int choosed_router=-1;
                for(unsigned int k=0;k<Nets_pos[i].half_router_line_v[l].cap_index.size();k++){
                    if(Nets_pos[i].half_router_line_v[l].cap_index[k]==1){
                       if(num_router_net_v[k]<=router_num){
                          choosed_router=k;
                          router_num = num_router_net_v[k];
                         }
                      }
                   }
                Nets_pos[i].line_v[choosed_router]=1;
               // Nets_pos[i].line_v[2*Cx-choosed_router]=1;
                num_router_net_v[choosed_router]=num_router_net_v[choosed_router]+1;
               // num_router_net_v[2*Cx-choosed_router]=num_router_net_v[2*Cx-choosed_router]+1;
               }
            }
       }
     }

  cout<<"broken down 11"<<endl;
//sample route methodology just for v neg
  for(unsigned int i=0;i<Nets_neg.size();i++){
      for(int k=0;k<=r;k++){Nets_neg[i].line_v.push_back(0);}
      int sum=0;
      for(unsigned int k=0;k<Nets_neg[i].routable_line_v.size();k++){sum=sum+Nets_neg[i].routable_line_v[k];}
      if(sum>0){
         //use the information of routable_line_v
         int router_num=2*Nets_neg.size();
         int choosed_router=-1;
         for(int j=0;j<=Cx;j++){
              if(Nets_neg[i].routable_line_v[j]==1){
                  if(num_router_net_v[j]<=router_num){
                     choosed_router=j;
                     router_num = num_router_net_v[j];
                    }
                }
            }
         if(1){
         Nets_neg[i].line_v[choosed_router]=1;
         Nets_neg[i].line_v[2*Cx-choosed_router]=1;
         num_router_net_v[choosed_router]=num_router_net_v[choosed_router]+1;
         num_router_net_v[2*Cx-choosed_router]=num_router_net_v[2*Cx-choosed_router]+1;
            }else{
            Nets_neg[i].line_v[2*Cx-choosed_router]=1;
            num_router_net_v[2*Cx-choosed_router]=num_router_net_v[2*Cx-choosed_router]+1;
            }
             
       }else{
         //use the information of half_routable_line_v
         for(unsigned int l=0;l<Nets_neg[i].half_router_line_v.size();l++){
             int found=0;
             for(unsigned int k=0;k<Nets_neg[i].half_router_line_v[l].cap_index.size();k++){
                 if(Nets_neg[i].half_router_line_v[l].cap_index[k]==1 and Nets_neg[i].line_v[k]==1){
                   found =1;
                   }
                }
             if(found ==0){
                int router_num=Nets_neg.size();
                int choosed_router=-1;
                for(unsigned int k=0;k<Nets_neg[i].half_router_line_v[l].cap_index.size();k++){
                    if(Nets_neg[i].half_router_line_v[l].cap_index[k]==1){
                       if(num_router_net_v[k]<=router_num){
                          choosed_router=k;
                          router_num = num_router_net_v[k];
                         }
                      }
                   }
                Nets_neg[i].line_v[choosed_router]=1;
                num_router_net_v[choosed_router]=num_router_net_v[choosed_router]+1;
               }
            }
       }
     }

  cout<<"broken down 12"<<endl;
   vector<int> num_line;
   for(unsigned int i=0;i<Nets_pos[0].line_v.size();i++){num_line.push_back(0);}
   for(unsigned int i=0;i<Nets_pos.size();i++){
       for(unsigned int j=0;j<Nets_pos[i].line_v.size();j++){
           num_line[j]=Nets_pos[i].line_v[j]+Nets_neg[i].line_v[j]+num_line[j];
          }
      }
   int max_num_ =0;
   for(unsigned int i=0;i<Nets_pos[0].line_v.size();i++){
        if(num_line[i]>max_num_){
           max_num_ = num_line[i];
          }
      }

  cout<<"broken down 13"<<endl;
   span_distance.first = (max_num_+1)*min_dis_x;
  cout<<span_distance.first<<endl;

  for(unsigned int i=0;i<Caps.size();i++){
      Caps[i].x = unit_cap_demension.first/2 +  Caps[i].index_x* (unit_cap_demension.first+span_distance.first);
     }

  cout<<"broken down 14"<<endl;
//route methdology in paper just for v
  //for one routable net

//route for the rest net (the same as sample router mathodology)

//generate the route phsical information
  //determine the start point and end point
  //for common cap both positive and negative
  //for dummy just negative is fine
  //finally return the port phsical information
  //adjust a uniform margin between the caps

//write gds file
  //based on the location of unit capacitor

  //and also give out the location of generated capacitor path to the centor database

}

int Placer_Router_Cap::found_neighbor(int j, net& pos, connection_set& temp_set){
  int found = -1;
  const auto& pcj = Caps[pos.cap_index[j]];
  for(unsigned int i=0;i<pos.cap_index.size();i++){
      auto& pci = Caps[pos.cap_index[i]];
      if(pci.access==0){
	  int adiffx = abs(pci.index_x -pcj.index_x);
	  int adiffy = abs(pci.index_y -pcj.index_y);
	  if((adiffx == 0 and adiffy==1) or (adiffy == 0 and adiffx==1)) {
	      pci.access = 1;
	      temp_set.cap_index.push_back(pos.cap_index[i]);
	      found = i;
	      found_neighbor(i, pos, temp_set);
	  }
      }
  } 
  if(found == -1){
      return -1;
  }else{
      return 1;
  }
}

void Placer_Router_Cap::addVia(net &temp_net, pair<double,double> &coord, const PnRDB::Drc_info &drc_info, const string& HV_via_metal, int HV_via_metal_index, int isPin){

  pair<double,double> via_coord;

  const auto& vm = drc_info.Via_model.at(HV_via_metal_index);

  temp_net.via.push_back(coord);                      
  temp_net.via_metal.push_back(HV_via_metal);

  if(drc_info.Metal_info.at(vm.LowerIdx).direct==1){
     //lower metal
     //start point
     via_coord.first = vm.LowerRect[0].x;
     via_coord.second = 0;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second;
     temp_net.start_conection_coord.push_back(via_coord);
     //end point
     via_coord.first = vm.LowerRect[1].x;
     via_coord.second = 0;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second; 
     temp_net.end_conection_coord.push_back(via_coord); 
     temp_net.Is_pin.push_back(isPin);
     temp_net.metal.push_back(drc_info.Metal_info[vm.LowerIdx].name);
                     
     //upper metal
     //start point
     via_coord.first = 0;
     via_coord.second = vm.UpperRect[0].y;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second;
     temp_net.start_conection_coord.push_back(via_coord);
     //end point
     via_coord.first = 0;
     via_coord.second = vm.UpperRect[1].y;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second; 
     temp_net.end_conection_coord.push_back(via_coord); 
     temp_net.Is_pin.push_back(isPin);
     temp_net.metal.push_back(drc_info.Metal_info.at(vm.UpperIdx).name); 
                                                
    }else{

     //lower metal
     //start point
     via_coord.first = 0;
     via_coord.second = vm.LowerRect[0].y;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second;
     temp_net.start_conection_coord.push_back(via_coord);
     //end point
     via_coord.first = 0;
     via_coord.second = vm.LowerRect[1].y;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second; 
     temp_net.end_conection_coord.push_back(via_coord); 
     temp_net.Is_pin.push_back(isPin);
     temp_net.metal.push_back(drc_info.Metal_info.at(vm.LowerIdx).name);
                          
     //upper metal
     //start point
     via_coord.first = vm.UpperRect[0].x;
     via_coord.second = 0;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second;
     temp_net.start_conection_coord.push_back(via_coord);
     //end point
     via_coord.first = vm.UpperRect[1].x;
     via_coord.second = 0;
     via_coord.first = via_coord.first + coord.first;
     via_coord.second = via_coord.second + coord.second; 
     temp_net.end_conection_coord.push_back(via_coord); 
     temp_net.Is_pin.push_back(isPin);
     temp_net.metal.push_back(drc_info.Metal_info.at(vm.UpperIdx).name);                     
     }


}

void Placer_Router_Cap::GetPhysicalInfo_router(const string& H_metal, int H_metal_index, const string& V_metal, int V_metal_index, const PnRDB::Drc_info &drc_info){

  int grid_offset;
  int height_cap = INT_MIN;

  for(unsigned int i=0;i<Caps.size();i++){
      if(Caps[i].y+unit_cap_demension.second/2 > height_cap){
          height_cap = Caps[i].y + unit_cap_demension.second/2;
        }
     }

  const auto& mH = drc_info.Metal_info.at(H_metal_index);

  int near_grid = ceil(height_cap/mH.grid_unit_y)*mH.grid_unit_y;

  grid_offset = (near_grid - height_cap)/2;


  string HV_via_metal;
  int HV_via_metal_index;

  if(H_metal_index>V_metal_index){
      HV_via_metal = V_metal;
      HV_via_metal_index = V_metal_index;
    }else{
      HV_via_metal = H_metal;
      HV_via_metal_index = H_metal_index;
    }


  pair<double,double> coord;
  pair<double,double> via_coord;

   //connection for trails
   int num_trail = Nets_pos[0].line_v.size();
   int routed_trail=0;
   vector<int> trails;


   for(int i=0;i<num_trail;i++){trails.push_back(0);}

   
   std::vector<


  for(unsigned int i=0;i<Caps.size();i++){
     Caps[i].access = 0;
  }
  routed_trail=0;
//for positive net
   for(unsigned int i=0;i<Nets_pos.size();i++){
       auto& n = Nets_pos[i];

      if(n.cap_index.size()==0){continue;}
      routed_trail=routed_trail+1;
      pair<double,double> first_coord;
      pair<double,double> end_coord;
      int first_lock=0;
      int end_close=0;
      for(unsigned int l=0;l<n.line_v.size();l++){
          if(n.line_v[l]==1){
              trails[l]=trails[l]+1;
              //connect to connection set and found the end point
              int max=-1;
              int max_cap_index=-1;
              int left_right = 0;
              int found = 0;
              for(unsigned int k=0;k<n.cap_index.size();k++){

                  if(Caps[n.cap_index[k]].index_x==l and Caps[n.cap_index[k]].access==0){
                      found = 1;
                      coord.first = Caps[n.cap_index[k]].x+ unit_cap_demension.first/2-shifting_x;
                      coord.second = Caps[n.cap_index[k]].y- unit_cap_demension.second/2+shifting_y;
                      // via coverage???
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                    
                      //
                      n.start_conection_coord.push_back(coord);
                      coord.first = Caps[n.cap_index[k]].x- unit_cap_demension.first/2-shifting_x-(span_distance.first-min_dis_x*trails[l]);
                      Caps[n.cap_index[k]].access = 1;
            
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);
                      n.metal.push_back(H_metal);

                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);                     
                      if(Caps[n.cap_index[k]].index_y>=max){
                         max=Caps[n.cap_index[k]].index_y;
                         max_cap_index = n.cap_index[k];
                         left_right = 0;
                        }
                    }else if(l-Caps[n.cap_index[k]].index_x==1 and Caps[n.cap_index[k]].access==0){
                      found = 1;
                      coord.first = Caps[n.cap_index[k]].x+ unit_cap_demension.first/2-shifting_x;
                      coord.second = Caps[n.cap_index[k]].y- unit_cap_demension.second/2+shifting_y;
                      
                      //
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                    
                      n.start_conection_coord.push_back(coord);
                      coord.second = Caps[n.cap_index[k]].y- unit_cap_demension.second/2+shifting_y-min_dis_y;
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);
                      n.metal.push_back(V_metal);

                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      n.start_conection_coord.push_back(coord);
                      coord.first = Caps[n.cap_index[k]].x+ unit_cap_demension.first/2-shifting_x+(min_dis_x*trails[l]);
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);

                      n.metal.push_back(H_metal);

                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      Caps[n.cap_index[k]].access = 1;
                      if(Caps[n.cap_index[k]].index_y>max){
                         max=Caps[n.cap_index[k]].index_y;
                         max_cap_index = n.cap_index[k];
                         left_right = 1;
                        }
                    }
                 }
                 if(found == 0){
                 for(unsigned int k=0;k<n.cap_index.size();k++){
                    if(l-Caps[n.cap_index[k]].index_x==1){
                      coord.first = Caps[n.cap_index[k]].x+ unit_cap_demension.first/2-shifting_x;
                      coord.second = Caps[n.cap_index[k]].y- unit_cap_demension.second/2+shifting_y;
                      
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);

                      n.start_conection_coord.push_back(coord);
                      coord.second = Caps[n.cap_index[k]].y- unit_cap_demension.second/2+shifting_y-min_dis_y;
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);
                      n.metal.push_back(V_metal);
                      
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);

                      n.start_conection_coord.push_back(coord);
                      coord.first = Caps[n.cap_index[k]].x+ unit_cap_demension.first/2-shifting_x+(min_dis_x*trails[l]);
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);

                      n.metal.push_back(H_metal);

                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      Caps[n.cap_index[k]].access = 1;

                      if(Caps[n.cap_index[k]].index_y>max){
                         max=Caps[n.cap_index[k]].index_y;
                         max_cap_index = n.cap_index[k];
                         left_right = 1;
                        }
                    }
                 }
                 }

                 coord.second = 0 - min_dis_y*routed_trail-2*min_dis_y-grid_offset;
                 addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,1);
                 
                 n.start_conection_coord.push_back(coord);

                 coord.second = -2*min_dis_y+shifting_y;
                 n.end_conection_coord.push_back(coord);
                 n.Is_pin.push_back(0);
                 n.metal.push_back(V_metal);

                 n.start_conection_coord.push_back(coord);
                 coord.second = Caps[max_cap_index].y- unit_cap_demension.second/2-left_right*min_dis_y+shifting_y;
                 n.end_conection_coord.push_back(coord);
                 n.Is_pin.push_back(0);
                 //
                 n.metal.push_back(V_metal);
                 //
                 if(first_lock==0){
                    first_coord.first = coord.first;
                    first_coord.second = 0 - min_dis_y*routed_trail-2*min_dis_y-grid_offset;
                    first_lock=1;
                 }else{
                    end_close=1;
                    end_coord.first = coord.first;;
                    end_coord.second = 0 - min_dis_y*routed_trail-2*min_dis_y-grid_offset;
                 }
            }
         }
       //connect to each trail
       if(first_lock==1 and end_close==1){
       n.start_conection_coord.push_back(first_coord);
       n.end_conection_coord.push_back(end_coord);
       n.Is_pin.push_back(1);

       n.metal.push_back(H_metal);
       }    
   }
    for(unsigned int i=0;i<Nets_pos.size();i++){
	auto& n = Nets_pos[i];

      //connection for each connection set
      for(unsigned int j=0;j<n.Set.size();j++){

              unsigned int end_flag = n.Set[j].cap_index.size();
              unsigned int index = 0;
              while(index<end_flag){
                     if(Caps[n.Set[j].cap_index[index]].access==1){
                        int found=0;
                        for(unsigned int k=0;k<end_flag;k++){
                            if((Caps[n.Set[j].cap_index[k]].index_y==Caps[n.Set[j].cap_index[index]].index_y and abs(Caps[n.Set[j].cap_index[k]].index_x-Caps[n.Set[j].cap_index[index]].index_x) ==1)and !(Caps[n.Set[j].cap_index[k]].access)){
                              Caps[n.Set[j].cap_index[k]].access=1;
                              coord.first = Caps[n.Set[j].cap_index[k]].x + unit_cap_demension.first/2-shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[k]].y - unit_cap_demension.second/2+shifting_y;  
                              

                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);

                              n.start_conection_coord.push_back(coord);
                              coord.first = Caps[n.Set[j].cap_index[index]].x + unit_cap_demension.first/2-shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[index]].y - unit_cap_demension.second/2+shifting_y;
                              n.end_conection_coord.push_back(coord);
                              n.Is_pin.push_back(0);
                              n.metal.push_back(H_metal);
                              //

                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                              
                              //
                              index = 0;
                              found = 1;
                             }else if((Caps[n.Set[j].cap_index[k]].index_x==Caps[n.Set[j].cap_index[index]].index_x and abs(Caps[n.Set[j].cap_index[k]].index_y-Caps[n.Set[j].cap_index[index]].index_y) ==1) and !(Caps[n.Set[j].cap_index[k]].access)){
                              Caps[n.Set[j].cap_index[k]].access=1;
                              coord.first = Caps[n.Set[j].cap_index[k]].x + unit_cap_demension.first/2-shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[k]].y - unit_cap_demension.second/2+shifting_y;  
                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);

                              n.start_conection_coord.push_back(coord);
                              coord.first = Caps[n.Set[j].cap_index[index]].x + unit_cap_demension.first/2-shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[index]].y - unit_cap_demension.second/2+shifting_y;
                              n.end_conection_coord.push_back(coord);
                              n.Is_pin.push_back(0);
                              n.metal.push_back(V_metal);

                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);

                              index = 0;
                              found = 1;
                             }
                           }
                           if(found==0){
                              index = index +1;
                             }
                       }else{
                        index=index+1;
                       }
                   }
              }
         }
     

//for negative net
  for(unsigned int i=0;i<Caps.size();i++){
     Caps[i].access = 0;
  }
   //connection for trails
   routed_trail=0;
   for(unsigned int i=0;i<Nets_neg.size();i++){
       auto& n = Nets_neg[i];

      if(n.cap_index.size()==0){continue;}
      routed_trail=routed_trail+1;
      pair<double,double> first_coord;
      pair<double,double> end_coord;
      int first_lock=0;
      int end_close=0;
      for(unsigned int l=0;l<n.line_v.size();l++){
          if(n.line_v[l]==1){
              trails[l]=trails[l]+1;
              //connect to connection set and found the end point
              int min=Caps.size();
              int min_cap_index=-1;
              int left_right = 0;
              int found = 0;
              for(unsigned int k=0;k<n.cap_index.size();k++){
                  if(Caps[n.cap_index[k]].index_x==l and Caps[n.cap_index[k]].access==0){
                      found = 1;
                      coord.first = Caps[n.cap_index[k]].x- unit_cap_demension.first/2+shifting_x;
                      coord.second = Caps[n.cap_index[k]].y+ unit_cap_demension.second/2-shifting_y;
                      
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      n.start_conection_coord.push_back(coord);
                      coord.first = Caps[n.cap_index[k]].x- unit_cap_demension.first/2 + shifting_x-(span_distance.first-min_dis_x*trails[l]);
                      Caps[n.cap_index[k]].access = 1;

                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);

                      n.metal.push_back(H_metal);

                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      if(Caps[n.cap_index[k]].index_y<=min){
                         min=Caps[n.cap_index[k]].index_y;
                         min_cap_index = n.cap_index[k];
                         left_right = 0;
                        }
                    }else if(l-Caps[n.cap_index[k]].index_x==1 and Caps[n.cap_index[k]].access==0){
                      found = 1;
                      coord.first = Caps[n.cap_index[k]].x- unit_cap_demension.first/2+shifting_x;
                      coord.second = Caps[n.cap_index[k]].y+ unit_cap_demension.second/2-shifting_y;
                      
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      n.start_conection_coord.push_back(coord);
                      coord.second = Caps[n.cap_index[k]].y+ unit_cap_demension.second/2+min_dis_y-shifting_y;
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);
                      n.metal.push_back(V_metal);

                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      n.start_conection_coord.push_back(coord);
                      coord.first = Caps[n.cap_index[k]].x+ unit_cap_demension.first/2+(min_dis_x*trails[l])+shifting_x;
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);
                      n.metal.push_back(H_metal);

                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      Caps[n.cap_index[k]].access = 1;

                      if(Caps[n.cap_index[k]].index_y<min){
                         min=Caps[n.cap_index[k]].index_y;
                         min_cap_index = n.cap_index[k];
                         left_right = 1;
                        }
                    }
                 }
                 if(found == 0){
                 for(unsigned int k=0;k<n.cap_index.size();k++){
		     if(l-Caps[n.cap_index[k]].index_x==1){
                      
                      coord.first = Caps[n.cap_index[k]].x- unit_cap_demension.first/2+shifting_x;
                      coord.second = Caps[n.cap_index[k]].y+ unit_cap_demension.second/2-shifting_y;
                      
                      //
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      //
                      n.start_conection_coord.push_back(coord);
                      coord.second = Caps[n.cap_index[k]].y+ unit_cap_demension.second/2+min_dis_y-shifting_y;
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);
                      n.metal.push_back(V_metal);
                      //
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      //
                      n.start_conection_coord.push_back(coord);
                      coord.first = Caps[n.cap_index[k]].x+ unit_cap_demension.first/2+(min_dis_x*trails[l])+shifting_x;
                      n.end_conection_coord.push_back(coord);
                      n.Is_pin.push_back(0);
                      n.metal.push_back(H_metal);
                      //
                      addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      //
                      Caps[n.cap_index[k]].access = 1;
                      if(Caps[n.cap_index[k]].index_y<min){
                         min=Caps[n.cap_index[k]].index_y;
                         min_cap_index = n.cap_index[k];
                         left_right = 1;
                        }
                    }
                 }
                 }
                 //connect from start to end for each trail 
                 int num_cap = Caps.size();
                 coord.second = Caps[num_cap-1].y+unit_cap_demension.second/2 + min_dis_y*routed_trail+2*min_dis_y + grid_offset;
                 addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,1);

                 n.start_conection_coord.push_back(coord);
                 coord.second = Caps[num_cap-1].y+unit_cap_demension.second/2+2*min_dis_y-shifting_y;
                 n.end_conection_coord.push_back(coord);
                 n.Is_pin.push_back(0);
                 n.metal.push_back(V_metal);
                 n.start_conection_coord.push_back(coord);
                 coord.second = Caps[min_cap_index].y+ unit_cap_demension.second/2+left_right*min_dis_y-shifting_y;
                 n.end_conection_coord.push_back(coord);
                 n.Is_pin.push_back(0);
                 n.metal.push_back(V_metal);
                 if(first_lock==0){
                    first_coord.first = coord.first;
                    first_coord.second = Caps[num_cap-1].y+unit_cap_demension.second/2 + min_dis_y*routed_trail+2*min_dis_y+grid_offset;
                    first_lock=1;
                 }else{
                    end_close=1;
                    end_coord.first = coord.first;
                    end_coord.second = Caps[num_cap-1].y+unit_cap_demension.second/2 + min_dis_y*routed_trail+2*min_dis_y+grid_offset;
                 }
            }
         }
       //connect to each trail
       if(first_lock==1 and end_close==1){
       n.start_conection_coord.push_back(first_coord);
       n.end_conection_coord.push_back(end_coord);
       n.Is_pin.push_back(1);
       //
       n.metal.push_back(H_metal);
       //
       
       }    
   }
  for(unsigned int i=0;i<Nets_neg.size();i++){
      auto& n = Nets_neg[i];
      //connection for each connection set
      for(unsigned int j=0;j<n.Set.size();j++){
              unsigned int end_flag = n.Set[j].cap_index.size();
              unsigned int index = 0;
              while(index<end_flag){
                     if(Caps[n.Set[j].cap_index[index]].access==1){
                        int found=0;
                        for(unsigned int k=0;k<end_flag;k++){
                            if((Caps[n.Set[j].cap_index[k]].index_y==Caps[n.Set[j].cap_index[index]].index_y and abs(Caps[n.Set[j].cap_index[k]].index_x-Caps[n.Set[j].cap_index[index]].index_x) ==1 and !(Caps[n.Set[j].cap_index[k]].access))){
                              Caps[n.Set[j].cap_index[k]].access=1;
                              coord.first = Caps[n.Set[j].cap_index[k]].x - unit_cap_demension.first/2+shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[k]].y + unit_cap_demension.second/2-shifting_y;  
                             
                              //
                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);

                              n.start_conection_coord.push_back(coord);
                              coord.first = Caps[n.Set[j].cap_index[index]].x - unit_cap_demension.first/2+shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[index]].y + unit_cap_demension.second/2-shifting_y;
                              n.end_conection_coord.push_back(coord);
                              n.Is_pin.push_back(0);
                              n.metal.push_back(H_metal);

                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                      
                      
                              index = 0;
                              found = 1;
                             }else if((Caps[n.Set[j].cap_index[k]].index_x==Caps[n.Set[j].cap_index[index]].index_x and abs(Caps[n.Set[j].cap_index[k]].index_y-Caps[n.Set[j].cap_index[index]].index_y) ==1 and !(Caps[n.Set[j].cap_index[k]].access))){
                              Caps[n.Set[j].cap_index[k]].access=1;
                              coord.first = Caps[n.Set[j].cap_index[k]].x - unit_cap_demension.first/2+shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[k]].y + unit_cap_demension.second/2-shifting_y;  
                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);

                              n.start_conection_coord.push_back(coord);
                              coord.first = Caps[n.Set[j].cap_index[index]].x - unit_cap_demension.first/2+shifting_x;
                              coord.second = Caps[n.Set[j].cap_index[index]].y + unit_cap_demension.second/2-shifting_y;
                              n.end_conection_coord.push_back(coord);
                              n.Is_pin.push_back(0);
                              n.metal.push_back(V_metal);

                              addVia(n,coord,drc_info,HV_via_metal,HV_via_metal_index,0);
                   
                              index = 0;
                              found = 1;
                             }
                           }
                           if(found==0){
                              index = index +1;
                             }
                       }else{
                        index=index+1;
                       }
                   }
              }
         }

}

extern
void JSONReaderWrite_subcells (string GDSData, long int& rndnum,
			       vector<string>& strBlocks, vector<int>& llx, vector<int>& lly,
			       vector<int>& urx, vector<int>& ury, json& mjsonStrAry);

extern
void JSONExtractUit (string GDSData, double& unit);

extern 
void addOABoundaries (json& jsonElements, int width, int height);

void
Placer_Router_Cap::fillPathBoundingBox (int *x, int* y,
					const pair<double,double> &start,
					const pair<double,double> &end,
					double width) {
    x[0] = offset_x;
    x[1] = offset_x;
    x[2] = offset_x;
    x[3] = offset_x;
    y[0] = offset_y;
    y[1] = offset_y;
    y[2] = offset_y;
    y[3] = offset_y;

    if (start.first == end.first) {
	// NO offset for Failure2
	if (start.second < end.second) {
	    x[0] += start.first;
	    x[1] += end.first;
	    x[2] += end.first;
	    x[3] += start.first;
             
	    y[0] += start.second;
	    y[1] += end.second;
	    y[2] += end.second;
	    y[3] += start.second;
	} else {
	    x[0] += end.first;
	    x[1] += start.first;
	    x[2] += start.first;
	    x[3] += end.first;
             
	    y[0] += end.second;
	    y[1] += start.second;
	    y[2] += start.second;
	    y[3] += end.second;
	}
    } else {
	if (start.first < end.first){
	    x[0] += start.first;
	    x[1] += start.first;
	    x[2] += end.first;
	    x[3] += end.first;
             
	    y[0] += start.second;
	    y[1] += start.second;
	    y[2] += end.second;
	    y[3] += end.second;

	} else {
	    x[0] += end.first;
	    x[1] += end.first;
	    x[2] += start.first;
	    x[3] += start.first;
             
	    y[0] += end.second;
	    y[1] += end.second;
	    y[2] += start.second;
	    y[3] += start.second;
	}
	

    }

    if (start.first == end.first) {
	x[0] -= width;
	x[1] -= width;
	x[2] += width;
	x[3] += width;
    } else {
	y[0] -= width;
	y[1] += width;
	y[2] += width;
	y[3] -= width;
    }

    x[4] = x[0];
    y[4] = y[0];
}

void
Placer_Router_Cap::WriteJSON (const string& fpath, const string& unit_capacitor, const string& final_gds, const PnRDB::Drc_info & drc_info, const string& opath) {
    //begin to write JSON file from unit capacitor to final capacitor file
    string gds_unit_capacitor = fpath+"/"+unit_capacitor+".gds";
    string topGDS_loc = opath+final_gds+".gds";
    string TopCellName = final_gds;
    double unitScale=2;
    JSONExtractUit (gds_unit_capacitor, unitScale);
    std::cout<<"Cap unitScale "<<unitScale<<std::endl;

    std::ofstream jsonStream;
    jsonStream.open (topGDS_loc + ".json");
    json jsonTop;

    jsonTop["header"] = 600;
    json jsonLibAry = json::array();
    json jsonLib;
    jsonLib["time"] = JSON_TimeTime();
    // DAK Overwrite to match
    jsonLib["time"] = {2019, 4, 24, 9, 46, 15, 2019, 4, 24, 9, 46, 15};
    double dbUnitUser=2*0.00025/unitScale;
    double dbUnitMeter=dbUnitUser/1e6;
    jsonLib["units"] = {dbUnitUser, dbUnitMeter};
    jsonLib["libname"] = "test";

    //what is this
    vector<string> uniGDS;
    for(unsigned int i=0;i<1;i++){
	uniGDS.push_back(gds_unit_capacitor);
    }

    json jsonStrAry = json::array();

    vector<string> strBlocks;
    vector<int> llx, lly, urx, ury;
    map<string,int> gdsMap2strBlock;
    long int rndnum=111;
    vector<string> strBlocks_Top;
    int idx=0;
    //writing unit capacitors??? confirm with jinhyun
    std::cout << "GDS CAP SUBCELL read of " << gds_unit_capacitor << std::endl;

    for(unsigned int i=0;i<uniGDS.size();i++) {
	json js;
	cout << "CAP GDS: Using JSON for subcells for now" << endl;
	JSONReaderWrite_subcells (gds_unit_capacitor, rndnum, strBlocks, llx,lly,urx,ury, js);
	for (json::iterator str = js.begin(); str != js.end(); ++str) {
	    jsonStrAry.push_back (*str);
	}

	if (strBlocks.size())
	    strBlocks_Top.push_back(strBlocks.back());
	else
	    std::cout << "ERROR: NO blocks returned from parsing " << gds_unit_capacitor << endl;
	gdsMap2strBlock.insert(make_pair(gds_unit_capacitor,idx));
	idx++;
    }
    //writing metals
    int x[5], y[5];

    json jsonStr;
    jsonLib["time"] = JSON_TimeTime();
    // DAK: Hack to match
    jsonStr["time"] = {2019, 4, 24, 9, 46, 15, 2019, 4, 24, 9, 46, 15};
    jsonStr["strname"] = TopCellName.c_str();
    json jsonElements = json::array();

    //for positive nets

    for(unsigned int i=0; i< Nets_pos.size(); i++){//for each net
	for(unsigned int j=0; j< Nets_pos[i].start_conection_coord.size();j++){ //for segment

	    const auto& mi = drc_info.Metal_info.at(drc_info.Metalmap.at(Nets_pos[i].metal[j]));
            int width = mi.width/2;
	    fillPathBoundingBox (x, y, Nets_pos[i].start_conection_coord[j],
				 Nets_pos[i].end_conection_coord[j], width);

	    for (int i = 0; i < 5; i++) {
		x[i] *= unitScale;
		y[i] *= unitScale;
	    }

	    json bound;
	    bound["type"] = "boundary";
	    bound["datatype"] = 0;
	    json xy = json::array();
	    for (size_t i = 0; i < 5; i++) {
		xy.push_back (x[i]);
		xy.push_back (y[i]);
	    }
	    bound["xy"] = xy;
	    bound["layer"] = getLayerMask (Nets_pos[i].metal[j], drc_info);
	    jsonElements.push_back (bound);
	}   
    }
  
    //for neg nets
    for(unsigned int i =0; i < Nets_neg.size(); i++) {//for each net
	for(unsigned int j = 0; j < Nets_neg[i].start_conection_coord.size(); j++) { //for segment
	    const auto& mi = drc_info.Metal_info.at(drc_info.Metalmap.at(Nets_neg[i].metal[j]));
            int width = mi.width/2;
	    fillPathBoundingBox (x, y, Nets_neg[i].start_conection_coord[j],
				 Nets_neg[i].end_conection_coord[j], width);

	    for (int i = 0; i < 5; i++) {
		x[i] *= unitScale;
		y[i] *= unitScale;
	    }

	    json bound;
	    bound["type"] = "boundary";
	    bound["datatype"] = 0;
	    json xy = json::array();
	    for (size_t i = 0; i < 5; i++) {
		xy.push_back (x[i]);
		xy.push_back (y[i]);
	    }
	    bound["xy"] = xy;
	    bound["layer"] = getLayerMask (Nets_neg[i].metal[j], drc_info);
	    jsonElements.push_back (bound);
	}
    }
  
    //wirting vias
    //for positive net
    //width = via_width[0];
    for (unsigned int i = 0; i < Nets_pos.size(); i++) {
	for (unsigned int j = 0; j < Nets_pos[i].via.size(); j++) {//the size of via needs to be modified according to different PDK
	    const auto& r = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_pos[i].via_metal[j])).ViaRect[1];
            int width = r.x;
 	    x[0]=Nets_pos[i].via[j].first - width+offset_x;
	    x[1]=Nets_pos[i].via[j].first - width+offset_x;
	    x[2]=Nets_pos[i].via[j].first + width+offset_x;
	    x[3]=Nets_pos[i].via[j].first + width+offset_x;
	    x[4]=x[0];
            width = r.y;
	    y[0]=Nets_pos[i].via[j].second - width+offset_y;
	    y[1]=Nets_pos[i].via[j].second + width+offset_y;
	    y[2]=Nets_pos[i].via[j].second + width+offset_y;
	    y[3]=Nets_pos[i].via[j].second - width+offset_y;
	    y[4]=y[0];
        
	    for (int i = 0; i < 5; i++) {
		x[i] *= unitScale;
		y[i] *= unitScale;
	    }
    
	    json bound;
	    bound["type"] = "boundary";
	    bound["datatype"] = 0;
	    json xy = json::array();
	    for (size_t i = 0; i < 5; i++) {
		xy.push_back (x[i]);
		xy.push_back (y[i]);
	    }
	    bound["xy"] = xy;
	    bound["layer"] = getLayerViaMask (Nets_pos[i].via_metal[j], drc_info);
	    jsonElements.push_back (bound);
	}
    }
    
    //for negative net
    for (unsigned int i = 0; i < Nets_neg.size(); i++) {
	for (unsigned int j = 0; j < Nets_neg[i].via.size(); j++) {//the size of via needs to be modified according to different PDK
	    const auto& r = drc_info.Via_model.at(drc_info.Metalmap.at(Nets_neg[i].via_metal[j])).ViaRect[1];
            int width = r.x;
	    x[0]=Nets_neg[i].via[j].first - width+offset_x;
	    x[1]=Nets_neg[i].via[j].first - width+offset_x;
	    x[2]=Nets_neg[i].via[j].first + width+offset_x;
	    x[3]=Nets_neg[i].via[j].first + width+offset_x;
	    x[4]=x[0];
            width = r.y;
	    y[0]=Nets_neg[i].via[j].second - width+offset_y;
	    y[1]=Nets_neg[i].via[j].second + width+offset_y;
	    y[2]=Nets_neg[i].via[j].second + width+offset_y;
	    y[3]=Nets_neg[i].via[j].second - width+offset_y;
	    y[4]=y[0];
        
	    for (int i = 0; i < 5; i++) {
		x[i] *= unitScale;
		y[i] *= unitScale;
	    }

	    json bound;
	    bound["type"] = "boundary";
	    bound["datatype"] = 0;
	    json xy = json::array();
	    for (size_t i = 0; i < 5; i++) {
		xy.push_back (x[i]);
		xy.push_back (y[i]);
	    }
	    bound["xy"] = xy;
	    bound["layer"] = getLayerViaMask (Nets_neg[i].via_metal[j], drc_info);
	    jsonElements.push_back (bound);
	}
    }
  
    //write orientation for each cap
    for (unsigned int i = 0; i < Caps.size(); i++) {
	json sref;
	sref["type"] = "sref";
	if (strBlocks_Top.size())
	    sref["sname"] = strBlocks_Top[0].c_str();
	else
	    cout << "ERROR: no block found to output from subcells" << endl;
	sref["strans"] = 0;
	sref["angle"] = 0.0;
	x[0] = unitScale*(Caps[i].x-unit_cap_demension.first/2+offset_x);
	y[0] = unitScale*(Caps[i].y-unit_cap_demension.second/2+offset_y);
     
	json xy = json::array();
	xy.push_back (x[0]);
	xy.push_back (y[0]);
	sref["xy"] = xy;
	jsonElements.push_back (sref);
    }

    addOABoundaries(jsonElements, unitScale * CheckOutBlock.width, unitScale * CheckOutBlock.height);
    jsonStr["elements"] = jsonElements;
    jsonStrAry.push_back (jsonStr);
    jsonLib["bgnstr"] = jsonStrAry;

    jsonLibAry.push_back(jsonLib);
    jsonTop["bgnlib"] = jsonLibAry;
    jsonStream << std::setw(4) << jsonTop;
    jsonStream.close();
    std::cout << "CAP GDS JSON FINALIZE " <<  unit_capacitor << std::endl;
}

void Placer_Router_Cap::Common_centroid_capacitor_aspect_ratio(const string& opath, const string& fpath, PnRDB::hierNode& current_node, PnRDB::Drc_info & drc_info, const map<string, PnRDB::lefMacro>& lefData, bool dummy_flag, bool aspect_ratio, int num_aspect){ //if aspect_ratio 1, then do CC with different aspect_ratio; Else not.


  for(unsigned int i = 0;i<current_node.Blocks.size();i++){
      auto& b = current_node.Blocks[i].instance.back();

      if(b.isLeaf == 1 and b.gdsFile ==""){
	   //this block must be CC
	   vector<int> ki;
	   vector<pair<string, string> > pin_names;
	   string unit_capacitor;
            string final_gds;
            pair<string, string> pins;
            for(unsigned int j=0;j<current_node.CC_Caps.size();j++){

                 std::cout<<"New CC 1 "<<j<<std::endl;
                 if(current_node.CC_Caps[j].CCCap_name == b.name){
                      ki = current_node.CC_Caps[j].size;
                      unit_capacitor = current_node.CC_Caps[j].Unit_capacitor;
                      final_gds = b.master;
                      for(unsigned int pin_index=0; pin_index <b.blockPins.size(); pin_index+=2){
                         pins.first = b.blockPins[pin_index].name;
                         pins.second = b.blockPins[pin_index+1].name;
                         pin_names.push_back(pins);
                      }
                      bool cap_ratio = current_node.CC_Caps[j].cap_ratio;
                      std::cout<<"New CC 2 "<<j<<std::endl;
                      vector<int> cap_r;
                      vector<int> cap_s;
                      if(cap_ratio){                        
			  cap_r.push_back(current_node.CC_Caps[j].cap_r);
			  cap_s.push_back(current_node.CC_Caps[j].cap_s);
                      }

                      std::cout<<"New CC 3 "<<j<<std::endl;
                      if(aspect_ratio){
                         int sum = 0;
                         for(unsigned int k=0;k<ki.size();k++){
			     sum = sum+ ki[k];
			 }
                         double temp_r = ceil(sqrt(sum));
                         double temp_s = ceil(sum/temp_r);
                         int aspect_num = num_aspect;
                         while(aspect_num > 0 and temp_r > 0){
                               
			     cap_r.push_back(temp_r);
			     cap_s.push_back(ceil(sum/temp_r));
			     cap_r.push_back(ceil(sum/temp_s));
			     cap_s.push_back(temp_s);

			     aspect_num = aspect_num - 2;
			     temp_r = temp_r - 1;
			     temp_s = temp_s + 1;

			 }
                                                  
		      }
                      //increase other aspect ratio
                      std::cout<<"New CC 4 "<<j<<std::endl;
                      std::cout<<"cap_r size "<<cap_r.size()<<std::endl;
                      for(unsigned int q=0;q<cap_r.size();q++){
                        std::cout<<"New CC 5 "<<j<<std::endl;
                        std::cout<<"New CC 6 "<<j<<std::endl;
                        std::cout<<"New CC 7 "<<j<<std::endl;
                        std::string cc_gds_file = final_gds+"_AspectRatio_"+to_string(cap_r[q])+"x"+to_string(cap_s[q]);
   
                        
                        std::cout<<"StartClean New CC"<<std::endl;
                        std::cout<<"q "<<q<<" cap_r[q] "<<cap_r[q]<<" cap_s[q] "<<cap_s[q]<<std::endl;
                        Placer_Router_Cap_clean();
                        std::cout<<"End Clean New CC"<<std::endl;

                        std::cout<<"Start New CC"<<std::endl;
                        Placer_Router_Cap_function(ki, pin_names, fpath, unit_capacitor, cc_gds_file, cap_ratio or aspect_ratio, cap_r[q], cap_s[q], drc_info, lefData, dummy_flag, opath);
                        std::cout<<"End New CC"<<std::endl;
                        PnRDB::block temp_block=CheckoutData();
                        //delete PRC;
                        
                        //std::cout<<"Start feed blocks"<<std::endl;

                        if(q!=0){
                            current_node.Blocks[i].instance.push_back(current_node.Blocks[i].instance[0]);
			}
			assert( (int)q == current_node.Blocks[i].instNum);
			current_node.Blocks[i].instNum++;
                        //feedback data
			auto& va = current_node.Blocks[i].instance.at(q);

			std::cout<<"Start feed blocks"<<std::endl;
			va.width = temp_block.width;
			va.height = temp_block.height;
			va.originBox = temp_block.originBox;
			va.originCenter = temp_block.originCenter;
			va.gdsFile = temp_block.gdsFile;
			va.orient = temp_block.orient;
			va.interMetals = temp_block.interMetals;
			va.interVias = temp_block.interVias;
			for(unsigned int k=0;k<va.blockPins.size();k++){
			    for(unsigned int l=0;l<temp_block.blockPins.size();l++){
				if(va.blockPins[k].name == temp_block.blockPins[l].name){    
				    va.blockPins[k].pinContacts.clear();   
				    for(unsigned int p=0;p<temp_block.blockPins[l].pinContacts.size();p++){
					va.blockPins[k].pinContacts.push_back(temp_block.blockPins[l].pinContacts[p]);
				    }
				}
			    }
			}
			WriteLef(va, cc_gds_file+".lef", opath);
			std::cout<<"End feed blocks"<<std::endl;
			continue;
		      } 
                   }
               }
         }
     }
}


void Placer_Router_Cap::PrintPlacer_Router_Cap(string outfile){
  cout<<"Placer-Router-Cap-Info: create gnuplot file"<<endl;
  ofstream fout;
  fout.open(outfile.c_str());

//set title
  fout<<"#Use this file as a script for gnuplot\n#(See http://www.gnuplot.info/ for details)"<<endl;
  fout<<"\nset title\" #capacitors= "<<Caps.size()<<" \""<<endl;
  fout<<"\nset nokey"<<endl;
  fout<<"#   Uncomment these two lines starting with \"set\""<<endl;
  fout<<"#   to save an EPS file for inclusion into a latex document"<<endl;
  fout<<"# set terminal postscript eps color solid 20"<<endl;
  fout<<"# set output \"result.eps\""<<endl<<endl;
  fout<<"#   Uncomment these two lines starting with \"set\""<<endl;
  fout<<"#   to save a PS file for printing"<<endl;
  fout<<"set term jpeg"<<endl;
  fout<<"set output \"result.jpg\""<<endl<<endl;

//set range
  fout<<"\nset xrange ["<<CheckOutBlock.originBox.LL.x-500<<":"<<CheckOutBlock.originBox.UR.x+500<<"]"<<endl;
  fout<<"\nset yrange ["<<CheckOutBlock.originBox.LL.y-500<<":"<<CheckOutBlock.originBox.UR.y+500<<"]"<<endl;

//set label for capacitors
  for(unsigned int i=0;i<Caps.size();i++){
       if(Caps[i].net_index!=-1){
       stringstream ss;
       ss<<Caps[i].net_index;
       string s1 = ss.str();
       string cap_name = "C_" + s1;
       fout<<"\nset label \""<<cap_name<<"\" at "<<Caps[i].x<<" , "<<Caps[i].y<<" center "<<endl;
       }else{
       string cap_name = "C_d";
       fout<<"\nset label \""<<cap_name<<"\" at "<<Caps[i].x<<" , "<<Caps[i].y<<" center "<<endl;
       }
       fout<<"\nset label \""<<i<<"\" at "<<Caps[i].x-unit_cap_demension.first/2<<" , "<<Caps[i].y-unit_cap_demension.second/2<<" center "<<endl;
     }

// plot capacitors
  fout<<"\nplot[:][:] \'-\' with lines linestyle 1,";
  for(unsigned int i=0;i<Nets_pos.size();i++){
     fout<<" \'-\' with lines linestyle "<<i+2<<",";
  }
  for(unsigned int i=0;i<Nets_neg.size();i++){
     fout<<" \'-\' with lines linestyle "<<i+2<<",";
  }
  fout<<endl<<endl;
  
  for(unsigned int i=0;i<Caps.size();i++){
      fout<<"\t"<<Caps[i].x-unit_cap_demension.first/2<<"\t"<<Caps[i].y-unit_cap_demension.second/2<<endl;
      fout<<"\t"<<Caps[i].x-unit_cap_demension.first/2<<"\t"<<Caps[i].y+unit_cap_demension.second/2<<endl;
      fout<<"\t"<<Caps[i].x+unit_cap_demension.first/2<<"\t"<<Caps[i].y+unit_cap_demension.second/2<<endl;
      fout<<"\t"<<Caps[i].x+unit_cap_demension.first/2<<"\t"<<Caps[i].y-unit_cap_demension.second/2<<endl;
      fout<<"\t"<<Caps[i].x-unit_cap_demension.first/2<<"\t"<<Caps[i].y-unit_cap_demension.second/2<<endl;
      fout<<endl;
     }
  if(Caps.size()>0){
  fout<<"\nEOF"<<endl; 
  }

// plot connection

  for(unsigned int i=0;i<Nets_pos.size();i++){
     for(unsigned int j=0;j<Nets_pos[i].start_conection_coord.size();j++){
     fout<<"\t"<<Nets_pos[i].start_conection_coord[j].first<<"\t"<<Nets_pos[i].start_conection_coord[j].second<<endl;
fout<<"\t"<<Nets_pos[i].end_conection_coord[j].first<<"\t"<<Nets_pos[i].end_conection_coord[j].second<<endl;
fout<<"\t"<<Nets_pos[i].start_conection_coord[j].first<<"\t"<<Nets_pos[i].start_conection_coord[j].second<<endl;
fout<<endl; 
        }
     if(Nets_pos.size()>0){  
        fout<<"\nEOF"<<endl;
        }
     }


  for(unsigned int i=0;i<Nets_neg.size();i++){
     for(unsigned int j=0;j<Nets_neg[i].start_conection_coord.size();j++){
     fout<<"\t"<<Nets_neg[i].start_conection_coord[j].first<<"\t"<<Nets_neg[i].start_conection_coord[j].second<<endl;
fout<<"\t"<<Nets_neg[i].end_conection_coord[j].first<<"\t"<<Nets_neg[i].end_conection_coord[j].second<<endl;
fout<<"\t"<<Nets_neg[i].start_conection_coord[j].first<<"\t"<<Nets_neg[i].start_conection_coord[j].second<<endl;
fout<<endl; 
        }
     if(Nets_neg.size()>0){  
       fout<<"\nEOF"<<endl;
      }
     }

  //fout<<endl<<"pause -1 \'Press any key\'";
  fout.close();
}
// Local Variables:
// c-basic-offset: 4
// End:

void Placer_Router_Cap::WriteLef(const PnRDB::block &temp_block, const string& file, const string& opath){

  std::ofstream leffile;
  string leffile_name = opath + file;

  leffile.open(leffile_name);

  double time = 2000;
  
  leffile<<"MACRO "<<temp_block.master<<std::endl;
  leffile<<"  ORIGIN 0 0 ;"<<std::endl;
  leffile<<"  FOREIGN "<<temp_block.master<<" 0 0 ;"<<std::endl;
  leffile<<"  SIZE "<< (double) temp_block.width/time<<" BY "<<(double) temp_block.height/time <<" ;"<<std::endl;

  //pins
  for(unsigned int i=0;i<temp_block.blockPins.size();i++){

      leffile<<"  PIN "<<temp_block.blockPins[i].name<<std::endl;
      leffile<<"    DIRECTION INOUT ;"<<std::endl;
      leffile<<"    USE SIGNAL ;"<<std::endl;
      //leffile<<"    DIRECTION "<<node.blockPins[i].type<<" ;"<<std::endl;
      //leffile<<"    USE "<<node.blockPins[i].use<<" 0 0 ;"<<std::endl;
      leffile<<"    PORT "<<std::endl;

      for(unsigned int j=0;j<temp_block.blockPins[i].pinContacts.size();j++){

         leffile<<"      LAYER "<<temp_block.blockPins[i].pinContacts[j].metal<<" ;"<<std::endl;
         leffile<<"        RECT "<<(double) temp_block.blockPins[i].pinContacts[j].originBox.LL.x/time<<" "<<(double) temp_block.blockPins[i].pinContacts[j].originBox.LL.y/time<<" "<<(double) temp_block.blockPins[i].pinContacts[j].originBox.UR.x/time<<" "<<(double) temp_block.blockPins[i].pinContacts[j].originBox.UR.y/time<<" ;"<<std::endl;

         }
      
      leffile<<"    END"<<std::endl;
      leffile<<"  END "<<temp_block.blockPins[i].name<<std::endl;  
      
 
     }

  leffile<<"  OBS "<<std::endl;
  for(unsigned int i=0;i<temp_block.interMetals.size();i++){

     
     leffile<<"  LAYER "<<temp_block.interMetals[i].metal<<" ;"<<std::endl;
     leffile<<"        RECT "<<(double) temp_block.interMetals[i].originBox.LL.x/time<<" "<<(double) temp_block.interMetals[i].originBox.LL.y/time<<" "<<(double) temp_block.interMetals[i].originBox.UR.x/time<<" "<<(double) temp_block.interMetals[i].originBox.UR.y/time<<" ;"<<std::endl;

     }
  leffile<<"  END "<<std::endl;

  leffile<<"END "<<temp_block.master<<std::endl;
  
  leffile.close();
}
