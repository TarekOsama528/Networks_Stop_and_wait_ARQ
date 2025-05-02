packet MyMessage {
    string M_Header;
    string M_Payload;
    string M_Trailer;
    int M_Type;
    float sending_time;
    int id;
    bool Error_Type; //0 for modified 1 for dup
    
	bool modification;    
    bool duplication;       
    bool delay;             
    bool loss;              
}
