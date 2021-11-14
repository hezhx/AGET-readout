void txt2root()
{
//file name : xinhao
	ifstream inputdata;
	inputdata.open("xinhao.txt");

	if (inputdata.fail())
	{
		cout << "sorry,couldn't read." << endl;
		exit(1);
	}

	//event is useless 
	//Int_t event = 0.;

	Int_t board = 0.;
	Int_t chip = 0.;
	Int_t channel = 0.;
	Float_t signal = 0.;
	// Float_t ES = 0;
	//Float_t I = 0.;


	Int_t lines = 0.;
	
    	string line;
    	ifstream myfile("xinhao.txt");

    	if(myfile.is_open()){
        	while(!myfile.eof()){
            	getline(myfile,line);
            	lines++;
        }
        myfile.close();
    }

	cout << "Lines: " << lines << endl;

	TFile *f = new TFile("xinhao.root", "RECREATE");
	TTree *T1 = new TTree("T1", "data from xinhao.txt");

	T1->Branch("board", &board, "board/I");	
	T1->Branch("chip", &chip, "chip/I");	
	T1->Branch("channel", &channel, "channel/I");	
	T1->Branch("signal", &signal, "signal/F");	
	// T1->Branch("ES", &ES, "ES/F");

	for (Int_t i = 0; i <= lines; i++)
	{
		inputdata 
			>> board//;
			>> chip
			>> channel
			>> signal;
			// >> ES;

		T1->Fill();
	}

	T1->Write();
	f->Close();
	inputdata.close();

    // TCanvas *c1 = new TCanvas("c1", "c1", 1200, 1000);
	// c1->cd();
	// T1->Draw("board");
	// c1->SaveAs("Signal.png");

}
