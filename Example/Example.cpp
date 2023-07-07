#include <iostream>
#include <DAQInterface.h>
#include <functional>

class AutomatedFunctions {   ///////////// class for automating functions from slowcontrol

 public:

  static DAQInterface DAQ_inter;

  static void new_event_func(){
    
    DAQ_inter.SendLog("Hello i received a new_event trigger");
    
    //do new event trigger action, maybe reload configuration from DB ??
    
  }

  
  static std::string start_func(){
    
    std::string tmp_msg="Device started";
    DAQ_inter.SendLog(tmp_msg);

    
    return tmp_msg;
    
  }
  
  static std::string voltage_change_func(){
        
    //update hardware voltages 1 & 3 with values from:
    // DAQ_inter.SC_vars["voltage_1"]->GetValue<float>()
    // DAQ_inter.SC_vars["voltage_3"]->GetValue<float>()
    
    std::string ret = "Changed voltages";
    return ret;
    
  }
  
private:
  
    AutomatedFunctions(){};
  
};

DAQInterface AutomatedFunctions::DAQ_inter;

int main(){


  ////////////////////////////// setup /////////////////////////////////

  std::string device_name = "my_device"; //name of my device
  std::string PGClient_configfile = "./PGClientConfig";
  std::string database_name = "daq";

  DAQInterface* DAQ_inter = &AutomatedFunctions::DAQ_inter;  // making local pointer of static DAQInterface
  DAQ_inter->Init(device_name, PGClient_configfile, database_name);  // initialiing DAQInterface 

  //////////////////////////////////////////////////////////////////////

  DAQ_inter->sc_vars["Status"]->SetValue("Initialising"); //setting status message

  ///////////////////////////////// logging alarms and triggers ///////////////////////////////

  DAQ_inter->SendLog("unimportant message", 9, device_name);   //sending log message to database specifing severity and device name
  DAQ_inter->SendLog("important message", 0, device_name);   //sending log message to database
  DAQ_inter->SendLog("normal message");   //sending log message to database defualt level 2 defult name is name passed to DAQInterface constructor

  DAQ_inter->SendAlarm("High current alarm"); // sending alarm message to database

  DAQ_inter->TriggerSubscribe("new_event",  AutomatedFunctions::new_event_func); // if the DAQ sends out a global "new_event" trigger will before the specifed function


  ///////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////// slow control variables /////////////////////
  
  //defining slow control values
  
  DAQ_inter->sc_vars["Status"]->SetValue("Ready"); 

  DAQ_inter->sc_vars.Add("Info",SlowControlElementType(INFO));
  DAQ_inter->sc_vars["Info"]->SetValue(" hello this is an information message ,.!{}[]<>?/`~'@\" ");
 
  DAQ_inter->sc_vars.Add("Start",SlowControlElementType(BUTTON), AutomatedFunctions::start_func);
  DAQ_inter->sc_vars["Start"]->SetValue(false);
  
  DAQ_inter->sc_vars.Add("Stop",SlowControlElementType(BUTTON));
  DAQ_inter->sc_vars["Stop"]->SetValue(false);
  
  DAQ_inter->sc_vars.Add("Quit",SlowControlElementType(BUTTON));
  DAQ_inter->sc_vars["Quit"]->SetValue(false);
  
  DAQ_inter->sc_vars.Add("power_on",SlowControlElementType(OPTIONS));
  DAQ_inter->sc_vars["power_on"]->AddOption("1");
  DAQ_inter->sc_vars["power_on"]->AddOption("0");
  DAQ_inter->sc_vars["power_on"]->SetValue("0");
  
 
  DAQ_inter->sc_vars.Add("voltage_1", SlowControlElementType(VARIABLE),  AutomatedFunctions::voltage_change_func);  //example variable with automated functions
  DAQ_inter->sc_vars["voltage_1"]->SetMin(0);
  DAQ_inter->sc_vars["voltage_1"]->SetMax(5000);
  DAQ_inter->sc_vars["voltage_1"]->SetStep(0.1);
  DAQ_inter->sc_vars["voltage_1"]->SetValue(3500.5);
  
  DAQ_inter->sc_vars.Add("voltage_2", SlowControlElementType(VARIABLE)); //example variable with no automated function
  DAQ_inter->sc_vars["voltage_2"]->SetMin(0);
  DAQ_inter->sc_vars["voltage_2"]->SetMax(5000);
  DAQ_inter->sc_vars["voltage_2"]->SetStep(10);
  DAQ_inter->sc_vars["voltage_2"]->SetValue(4000);
  
  DAQ_inter->sc_vars.Add("voltage_3", SlowControlElementType(VARIABLE),  AutomatedFunctions::voltage_change_func);
  DAQ_inter->sc_vars["voltage_3"]->SetMin(0);
  DAQ_inter->sc_vars["voltage_3"]->SetMax(5000);
  DAQ_inter->sc_vars["voltage_3"]->SetStep(0.1);
  DAQ_inter->sc_vars["voltage_3"]->SetValue(3800);
  

  ////////////////////////////////////////////////////////////////////


  ////////////////////////////// configuration ///////////////////////
  
  Store configuration; //making ascii store to hold and retreive configuration variables
  std::string config_json=""; //string to hold json configuration
  unsigned int version=0; //version of configuration to get  

  float voltage_2 = DAQ_inter->sc_vars["voltage_2"]->GetValue<float>(); // example of indirect assignment of local variable from slow control collection
  DAQ_inter->sc_vars["voltage_2"]->GetValue(voltage_2); // example of direct assignment of local variable from slow control collection

  bool power_on = false;

  bool ok = DAQ_inter->GetConfig(config_json, version); //get configuration from database
  
  if(!ok || config_json==""){
    
    configuration.Set("power_on", power_on); // configuration can be passed any variable type directly
    configuration.Set("voltage_1", DAQ_inter->sc_vars["voltage_2"]->GetValue<float>()); // example using direct slow control variable
    configuration.Set("voltage_2", voltage_2); // example using local variable
    configuration.Set("voltage_3", 4000); // example using direct value
    
    configuration>>config_json; //output current configuration to json
    
    //std::cout<<"sending new config_json: '"<<config_json<<"'"<<std::endl;
  
    DAQ_inter->SendConfig(config_json); //uplaod configuration to database
    
  } else{

    configuration.JsonParser(config_json); //loading json into variable store
    
    configuration.Print(); //print the current configuration
  
    configuration.Get("power_on", power_on); // example getting local variable
    DAQ_inter->sc_vars["power_on"]->SetValue(power_on);
 
    DAQ_inter->sc_vars["voltage_1"]->SetValue(configuration.Get<float>("voltage_1")); //exmaple of assinging slow control value directly from configuration
    
    if(configuration.Get("voltage_2", voltage_2) == false) voltage_2 = 2000;  //set defualt value if not in configuration;
    DAQ_inter->sc_vars["voltage_2"]->SetValue(voltage_2);
    
    float voltage_3=0;
    if(!configuration.Get("voltage_3", voltage_3)) DAQ_inter->SendLog("voltage3 not set", 0, device_name); //sends log message if not in configuration
    
  }
  
  ////////////////////////////////////////////////////////////////////
  

  //////////////////////////////// creating monitoring store //////////////

  Store monitoring_data; // sorage object for monitoring vales;

  /////////////////////////////////////////////////////////////////



  /////////////////////////// program operation //////////////////////
  
  bool running=true;

  while(running){ /// program loop

    running=(!DAQ_inter->sc_vars["Quit"]->GetValue<bool>()); 

    bool started=false;
    
    

    //////////////////////////// Example of checking values manually, note: all below secion could also be done in automated.start_func() /////////

    if(DAQ_inter->sc_vars["Start"]->GetValue<bool>()){ // value can be used directly from slow control collection

      started = DAQ_inter->sc_vars["Start"]->GetValue<bool>(); // example of value assinged to variable
      DAQ_inter->sc_vars["Start"]->GetValue(started); // example of direct assingment using getter

      DAQ_inter->sc_vars["Status"]->SetValue("Running");
      DAQ_inter->sc_vars["Start"]->SetValue(false);  // resetting slow control value after use.
    
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
    while(started){ // program operation loop

      DAQ_inter->sc_vars.TriggerSend("test"); // example of sending trigger to all DAQ devices

      running=(!DAQ_inter->sc_vars["Quit"]->GetValue<bool>()); 

      if(DAQ_inter->sc_vars["Stop"]->GetValue<bool>() || !running){
	started =false;
	DAQ_inter->sc_vars["Status"]->SetValue("Stopped");
	DAQ_inter->sc_vars["Stop"]->SetValue(false);
	DAQ_inter->sc_vars["Start"]->SetValue(false);
      }
      
      
      ///////////////////////////////////////// monitoring usage ////////////////////

      monitoring_data.Delete(); // Deleting all current monitoring values. note: not needed if you want values to persist as they can be overwritten by calling set again

      
      monitoring_data.Set("temp_1", 30+(rand()%100)/100.);  // values recorded in monitoring store
      monitoring_data.Set("temp_2", 28+(rand()%100)/100.);
      monitoring_data.Set("temp_3", 18+(rand()%100)/100.);
      monitoring_data.Set("current_1", rand()%10/2.);
      monitoring_data.Set("current_2", rand()%10/2.);
      monitoring_data.Set("current_3", rand()%10/2.);
      monitoring_data.Set("voltage_1", DAQ_inter->sc_vars["voltage_1"]->GetValue<float>());
      monitoring_data.Set("voltage_2", DAQ_inter->sc_vars["voltage_2"]->GetValue<float>());
      monitoring_data.Set("voltage_3", DAQ_inter->sc_vars["voltage_3"]->GetValue<float>());
      monitoring_data.Set("power_on", DAQ_inter->sc_vars["power_on"]->GetValue<int>());
      
      std::string monitoring_json="";
      monitoring_data>>monitoring_json; /// prducing monitoring json 
     
      DAQ_inter->SendMonitoringData(monitoring_json);

      //////////////////////////////////////////////////////////////////////////////////////////


      //////////////////////////////  using and getting slow control values /////////////// 
      
      DAQ_inter->sc_vars["power_on"]->GetValue(power_on);
          
      if(!power_on){
	DAQ_inter->sc_vars["voltage_1"]->SetValue(0.0); 
	DAQ_inter->sc_vars["voltage_2"]->SetValue(0.0);
	DAQ_inter->sc_vars["voltage_3"]->SetValue(0.0);
      }


      // voltage 1 & 3 are changed automatically via automated_functions.voltage_change, but here is an example for voltage 2 of doing it manually

      if(DAQ_inter->sc_vars["voltage_2"]->GetValue<float>() != voltage_2 ){ // comaring with last known voltage using local variable
	DAQ_inter->sc_vars["voltage_2"]->GetValue(voltage_2);  //updating local voltage variable
	
	// change voltage

      }

    
    ////////////////////////////////    

      usleep(100);

    } // end of operation loop


    usleep(1000);
  
  } // end of program loop



  DAQ_inter->sc_vars["Status"]->SetValue("Terminated");

  
  return 0;

}