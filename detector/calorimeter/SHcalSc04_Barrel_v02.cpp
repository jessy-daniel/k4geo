//====================================================================
//  DD4hep Geometry driver for HcalBarrel
//--------------------------------------------------------------------
//  S.Lu, DESY
//  $Id:  $
//====================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "XML/Layering.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSimExceptions.h"

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;
using namespace DDSim ;

static Ref_t create_detector(LCDD& lcdd, xml_h element, SensitiveDetector sens)  {
  
  //double boundarySafety = 0.0001;

  xml_det_t   x_det       = element;
  string      det_name    = x_det.nameStr();
  DetElement  sdet( det_name,x_det.id() );
  
  // --- create an envelope volume and position it into the world ---------------------
  
  Volume envelope = XML::createPlacedEnvelope( lcdd,  element , sdet ) ;
  
  if( lcdd.buildType() == BUILD_ENVELOPE ) return sdet ;

  //-----------------------------------------------------------------------------------

  xml_comp_t    x_staves          = x_det.staves();
  Material      stavesMaterial    = lcdd.material(x_staves.materialStr());
  Material      air               = lcdd.air();

  PlacedVolume pv;

  sens.setType("calorimeter");

  // Some verbose output
  cout << " \n\n\n CREATE DETECTOR: SHcalSC04_Barrel" << endl;




  //====================================================================
  //
  // Read all the constant from ILD_o1_v05.xml
  // Use them to build HcalBarrel
  //
  //====================================================================
  double      Hcal_inner_radius   = lcdd.constant<double>("Hcal_inner_radius");
  double      Hcal_outer_radius   = lcdd.constant<double>("Hcal_outer_radius");
  double      Hcal_half_length    = lcdd.constant<double>("Hcal_half_length");
  int         Hcal_inner_symmetry = lcdd.constant<int>("Hcal_inner_symmetry");
  int         Hcal_outer_symmetry = 0; // Fixed shape for Tube, and not allow to modify from compact xml.

  double      Hcal_radiator_thickness          = lcdd.constant<double>("Hcal_radiator_thickness");
  double      Hcal_chamber_thickness           = lcdd.constant<double>("Hcal_chamber_thickness");
  double      Hcal_back_plate_thickness        = lcdd.constant<double>("Hcal_back_plate_thickness");
  double      Hcal_lateral_plate_thickness     = lcdd.constant<double>("Hcal_lateral_structure_thickness");
  //double      Hcal_stave_gaps                  = lcdd.constant<double>("Hcal_stave_gaps");
  double      Hcal_modules_gap                 = lcdd.constant<double>("Hcal_modules_gap"); 
  double      Hcal_layer_air_gap               = lcdd.constant<double>("Hcal_layer_air_gap");
  double      Hcal_cells_size                  = lcdd.constant<double>("Hcal_cells_size");

  int         Hcal_nlayers                     = lcdd.constant<int>("Hcal_nlayers");

  double      TPC_outer_radius               = lcdd.constant<double>("TPC_outer_radius");
  std::cout << " ***********TPC_outer_radius " << TPC_outer_radius  << std::endl ;

  double      Ecal_outer_radius               = lcdd.constant<double>("Ecal_outer_radius");
  std::cout << " ***********Ecal_outer_radius " <<  Ecal_outer_radius << std::endl ;



  //========== fill data for reconstruction ============================
  DDRec::LayeredCalorimeterData* caloData = new DDRec::LayeredCalorimeterData ;
  caloData->layoutType = DDRec::LayeredCalorimeterData::BarrelLayout ;
  caloData->inner_symmetry = Hcal_inner_symmetry  ;
  caloData->outer_symmetry = Hcal_outer_symmetry  ;
  caloData->phi0 = 0 ; // fg: also hardcoded below 

  /// extent of the calorimeter in the r-z-plane [ rmin, rmax, zmin, zmax ] in mm.
  caloData->extent[0] = Hcal_inner_radius ;
  caloData->extent[1] = Hcal_outer_radius ;
  caloData->extent[2] = 0. ; // Barrel zmin is "0" by default.
  caloData->extent[3] = Hcal_half_length ;

  //====================================================================
  //
  // general calculated parameters
  //
  //====================================================================

  double Hcal_total_dim_y   = Hcal_nlayers * (Hcal_radiator_thickness + Hcal_chamber_thickness) 
                            + Hcal_back_plate_thickness;

  double Hcal_normal_dim_z  = (2 * Hcal_half_length - Hcal_modules_gap)/2.;

  //only the middle has the steel plate.
  double Hcal_regular_chamber_dim_z = Hcal_normal_dim_z - Hcal_lateral_plate_thickness;
  
  double Hcal_cell_dim_x            = Hcal_cells_size;
  double Hcal_cell_dim_z            = Hcal_regular_chamber_dim_z / floor (Hcal_regular_chamber_dim_z/Hcal_cell_dim_x);

 
  // ========= Create Hcal Barrel stave   ====================================
  //  It will be the volume for palcing the Hcal Barrel Chamber(i.e. Layers).
  //  Itself will be placed into the world volume.
  // ==========================================================================
 
  //double chambers_y_off_correction = 0.;
 
  // stave modules shaper parameters
  //double BHX  = (Hcal_bottom_dim_x + Hcal_stave_gaps)/2.;
  //double THX  = (Hcal_total_dim_y + Hcal_inner_radius)*tan(M_PI/Hcal_inner_symmetry);
  //double YXH  = Hcal_total_dim_y / 2.;
  //double DHZ  = (Hcal_normal_dim_z - Hcal_lateral_plate_thickness) / 2.;

  //Trapezoid stave_shaper(  THX, BHX, DHZ, DHZ, YXH);

  //Tube solidCaloTube(0, Hcal_outer_radius, DHZ+boundarySafety);
  
  //RotationZYX rot(0,0,M_PI/2.);

  //Rotation3D rot3D(rot);
  //Position xyzVec(0,0,(Hcal_inner_radius + Hcal_total_dim_y / 2.));
  //Transform3D tran3D(rot3D,xyzVec);

  //IntersectionSolid barrelModuleSolid(stave_shaper, solidCaloTube, tran3D);

  //Volume  EnvLogHcalModuleBarrel(det_name+"_module",barrelModuleSolid,stavesMaterial);
  Assembly EnvLogHcalModuleBarrel(det_name+"_module");

  EnvLogHcalModuleBarrel.setAttributes(lcdd,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());



  //stave modules lateral plate shaper parameters
  //double BHX_LP  = BHX;
  //double THX_LP  = THX;
  //double YXH_LP  = YXH;

  //build lateral palte here to simulate lateral plate in the middle of barrel.
  //double DHZ_LP  = Hcal_lateral_plate_thickness/2.0; 

  //Trapezoid stave_shaper_LP(THX_LP, BHX_LP, DHZ_LP, DHZ_LP, YXH_LP);

  //Tube solidCaloTube_LP(0, Hcal_outer_radius, DHZ_LP+boundarySafety);

  //IntersectionSolid Module_lateral_plate(stave_shaper_LP, solidCaloTube_LP, tran3D);

  //Volume  EnvLogHcalModuleBarrel_LP(det_name+"_Module_lateral_plate",Module_lateral_plate,stavesMaterial);
  //Assembly EnvLogHcalModuleBarrel_LP(det_name+"_Module_lateral_plate");

  //EnvLogHcalModuleBarrel_LP.setAttributes(lcdd,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());



#ifdef SHCALSC04_DEBUG
  std::cout<< " ==> Hcal_outer_radius: "<<Hcal_outer_radius <<std::endl;
#endif





  //====================================================================
  //
  // Chambers in the HCAL BARREL
  //
  //====================================================================
  // Build Layer Chamber fill with air, which include the tolarance space at the two side
  // place the slice into the Layer Chamber
  // Place the Layer Chamber into the Stave module
  // place the Stave module into the asembly Volume
  // place the module middle lateral plate into asembly Volume

  double xShift = 0.;


  //-------------------- start loop over HCAL layers ----------------------
  double layerThickness =  Hcal_radiator_thickness +Hcal_chamber_thickness;
  double AngleRatio=tan(M_PI/Hcal_inner_symmetry);//"k", updated for flexible symmetry
  double d_InnerOctoSize=AngleRatio*Hcal_inner_radius;//"d"

  for (int layer_id = 1; layer_id <= Hcal_nlayers; layer_id++)
    {
 
      double yn = sqrt(Hcal_outer_radius*Hcal_outer_radius -
		       (Hcal_inner_radius + layer_id*layerThickness)*(Hcal_inner_radius + layer_id*layerThickness));

      //updated for flexible symmetry
      double Yn = d_InnerOctoSize - layer_id*layerThickness/tan(M_PI*2.0/Hcal_inner_symmetry);

      double halfX = Hcal_chamber_thickness/2.;
      double halfY = (yn+Yn)/2.-Hcal_layer_air_gap;
      double halfZ = Hcal_regular_chamber_dim_z / 2.;

      xShift = (yn-Yn)/2.;

      // Check the space for next layer in the envelope
      if( Yn <= 0 && yn <= xShift) continue; // No space for further layer anymore.

      //--------------------------------------------------------------------------------
      //  build chamber box, with the calculated dimensions 
      //-------------------------------------------------------------------------------
      cout <<" \n Start to build layer chamber "<<endl;
      cout <<"layer_id: "<< layer_id <<endl;
      cout <<" chamber x:y:z:   "<<halfZ*2.<<":"<<halfY*2.<<":"<<halfX*2.<<endl;

      Box ChamberSolid((halfY + Hcal_layer_air_gap), //Add gap at two sides, do not need to build air gaps individualy.
		       halfZ,   //z attention!
		       halfX); //y attention!

      string ChamberLogical_name      = det_name+_toString(layer_id,"_layer%d");

      Volume ChamberLogical(ChamberLogical_name, ChamberSolid, air);   

      double layer_thickness = halfX*2.;


      //====================================================================
      // Create Hcal Barrel Chamber without radiator
      // Place into the Hcal Barrel stave, after each radiator 
      //====================================================================
      xml_coll_t c(x_det,_U(layer));
      xml_comp_t   x_layer = c;
      string layer_name      = det_name+_toString(layer_id,"_layer%d");
      
      // Create the slices (sublayers) within the Hcal Barrel Chamber.
      double slice_pos_z = -(layer_thickness/2.);
      int slice_number = 0;
      for(xml_coll_t k(x_layer,_U(slice)); k; ++k)  {
	xml_comp_t x_slice = k;
	string   slice_name      = layer_name + _toString(slice_number,"_slice%d");
	double   slice_thickness = x_slice.thickness();
	Material slice_material  = lcdd.material(x_slice.materialStr());
	DetElement slice(layer_name,_toString(slice_number,"slice%d"),x_det.id());
	
	slice_pos_z += slice_thickness/2.;
	
	// Slice volume & box
	Volume slice_vol(slice_name,Box(halfY,halfZ,halfX),slice_material);
	
	if ( x_slice.isSensitive() ) {
	  slice_vol.setSensitiveDetector(sens);
	}
	// Set region, limitset, and vis.
	slice_vol.setAttributes(lcdd,x_slice.regionStr(),x_slice.limitsStr(),x_slice.visStr());
	// slice PlacedVolume
	PlacedVolume slice_phv = ChamberLogical.placeVolume(slice_vol,Position(0.,0.,slice_pos_z));
	if ( x_slice.isSensitive() ) {
	  int slice_id  = (layer_id > Hcal_nlayers)? 1:-1;
	  slice_phv.addPhysVolID("layer",layer_id).addPhysVolID("slice",slice_id);
	  cout<<"  layer_id:  "<< layer_id<<"   slice_id:  "<<slice_id <<endl;
	}
	
	slice.setPlacement(slice_phv);
	// Increment x position for next slice.
	slice_pos_z += slice_thickness/2.;
	// Increment slice number.
	++slice_number;             
      }


      //---------------------------  Chamber Placements -----------------------------------------

      double chamber_x_offset, chamber_y_offset, chamber_z_offset;
      chamber_x_offset = xShift;
      
      chamber_z_offset = 0;


      chamber_y_offset = -(-Hcal_total_dim_y/2. 
			   + (layer_id-1) *(Hcal_chamber_thickness + Hcal_radiator_thickness)
			   + Hcal_radiator_thickness + Hcal_chamber_thickness/2.);


      pv =  EnvLogHcalModuleBarrel.placeVolume(ChamberLogical,
					       Position(chamber_x_offset,
							chamber_z_offset,
							chamber_y_offset ));
      //chamber_y_offset + chambers_y_off_correction));
      

      
      //-----------------------------------------------------------------------------------------
      DDRec::LayeredCalorimeterData::Layer caloLayer ;
      
      //caloLayer.distance = Hcal_inner_radius + Hcal_total_dim_y/2.0 + chamber_y_offset + chambers_y_off_correction ;
      caloLayer.distance = Hcal_inner_radius + Hcal_total_dim_y/2.0 + chamber_y_offset ;
      caloLayer.thickness = Hcal_chamber_thickness + Hcal_radiator_thickness ;
      caloLayer.absorberThickness = Hcal_radiator_thickness ;
      caloLayer.cellSize0 = Hcal_cell_dim_z ;
      caloLayer.cellSize1 = Hcal_cell_dim_x ;
      
      caloData->layers.push_back( caloLayer ) ;
      //-----------------------------------------------------------------------------------------

      
    }//end loop over HCAL nlayers;
  





  //====================================================================
  // Place HCAL Barrel stave module into the envelope
  //====================================================================
  //double stave_phi_offset,  module_z_offset,  lateral_plate_z_offset;
  double stave_phi_offset,  module_z_offset ;
  
  double Y = Hcal_inner_radius + Hcal_total_dim_y / 2.;

  stave_phi_offset = M_PI/Hcal_inner_symmetry -M_PI/2.;


  //-------- start loop over HCAL BARREL staves ----------------------------

  for (int stave_id = 1;
       stave_id <=Hcal_inner_symmetry;
       stave_id++)
    {
      module_z_offset = - (Hcal_normal_dim_z + Hcal_modules_gap + Hcal_lateral_plate_thickness)/2.;
      //lateral_plate_z_offset = - (Hcal_lateral_plate_thickness + Hcal_modules_gap)/2.;

      double phirot = stave_phi_offset;
      RotationZYX rot(0,phirot,M_PI*0.5);
      Rotation3D rot3D(rot);

      for (int module_id = 1;
         module_id <=2;
         module_id++)
	{
	  Position xyzVec(-Y*sin(phirot), Y*cos(phirot), module_z_offset);
	  Transform3D tran3D(rot3D,xyzVec);
	  
	  // Place Hcal Barrel volume into the envelope volume
	  pv = envelope.placeVolume(EnvLogHcalModuleBarrel,tran3D);
	  pv.addPhysVolID("stave",stave_id);
	  pv.addPhysVolID("module",module_id);

	  //Position xyzVec_LP(-Y*sin(phirot), Y*cos(phirot),lateral_plate_z_offset);
	  //Transform3D tran3D_LP(rot3D,xyzVec_LP);
	  //pv = envelope.placeVolume(EnvLogHcalModuleBarrel_LP,tran3D_LP);

	  module_z_offset = - module_z_offset;
	  //lateral_plate_z_offset = - lateral_plate_z_offset;
	}


      stave_phi_offset -=  M_PI*2.0/Hcal_inner_symmetry;
    }  //-------- end loop over HCAL BARREL staves ----------------------------


  sdet.addExtension< DDRec::LayeredCalorimeterData >( caloData ) ;

 
  return sdet;

}

DECLARE_DETELEMENT(SHcalSc04_Barrel_v02, create_detector)