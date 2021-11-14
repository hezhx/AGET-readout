#include <string>
#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <numeric>
#include <vector>

using namespace std;

int hex2dec(string a);
double mean(double *data, double n);
double stddev(double *data, double n);
short high_low_exchange(short data1);

const int N = 4000000;
unsigned short Packet0[N];

unsigned long timestamp[8192] = {0};
unsigned long event_ID[8192] = {0};
unsigned long end_flag[10][8192] = {0};
unsigned long card_info[6][8192][518] = {0};
// double b[8192] = {0.}; //b, c, ch, sig_max, sig_baseline, sig;
// double c[8192] = {0.};
// double ch[8192] = {0.};
// double sig_max[8192] = {0.};
// double sig_baseline[8192] = {0.};
// double sig[8192] = {0.};
// double sig_1[5][8192][512];


// int Analysis(const char file[], int board_ID, int channel_ID)
int Analysis(const char file[])
{
    // cout << "Hello World!\n";
    gROOT->Reset();
    // open the dat(hex data file)
    char filename[256] = "";
    // cout << filename << endl;
    memset(filename, 0, sizeof(filename));
    // printf("Please input file : ");
    // scanf("%s", filename);
    // cout << "Please input file : ";
    // cin >> filename;
    // filename = file;
    strcpy(filename, file);
    //数组名只是数组第一个元素的地址
    // cout << &filename << endl;

    FILE *fid = fopen(filename, "rb");
    if (fid == NULL)
    {
        printf("Open file : %s failed.\n", filename);
        exit(1);
    }
    else
    {
        printf("File : %s opened.\n", filename);
    }

    memset(Packet0, 0, sizeof(Packet0));                             //????????
    auto file_data = fread(Packet0, sizeof(unsigned short), N, fid); //?
    const int M = file_data;
    unsigned short Packet[M];
    for (int i = 0; i < M; i++)
    {
        Packet[i] = high_low_exchange(Packet0[i]);
    }
    fclose(fid);
    //    auto file_data_amount = floor(sizeof(Packet));

    cout << "file data : " << file_data << endl;
    //    cout << "file data amount : " << file_data_amount << endl;

    // for (int i = 0; i < 50; i++)
    // {
    //     cout << "Packet " << i << " : " << Packet[i] << "\n";
    // }
    unsigned short START_OF_PACKET = hex2dec("ac0f");       // 44047
    unsigned short take_SOE_bit = hex2dec("4000");          // 16384
    unsigned short take_EOE_bit = hex2dec("2000");          // 8192
    unsigned short take_Packet_Size_bits = hex2dec("1fff"); // 8191
    unsigned short take_SOURCE_ID_bits = hex2dec("001f");   // 31
    unsigned short take_Card_num_bits = hex2dec("3e00");    // 15872
    unsigned short take_Chip_num_bits = hex2dec("0180");    // 384
    unsigned short take_Chn_num_bits = hex2dec("007f");     // 127
    unsigned short take_ADC_code_bits = hex2dec("0fff");    // 4095

    // cout << START_OF_PACKET << "\n"
    //      << take_SOE_bit << "\n"
    //      << take_EOE_bit << "\n"
    //      << take_Packet_Size_bits << "\n"
    //      << take_SOURCE_ID_bits << "\n"
    //      << take_Card_num_bits << "\n"
    //      << take_Chip_num_bits << "\n"
    //      << take_Chn_num_bits << "\n"
    //      << take_ADC_code_bits << "\n";

    int current_position = 0;
    int current_packet_length = 0;
    int encode_num = 1;

    int card_info_num[24] = {0};
    int end_flag_num[24] = {0};
    // 1 DCM can connect to 24 FECs at most
    for (int j = 0; j < 24; j++)
    {
        card_info_num[j] = 1; // 1*24 "one" matrix; 1 DCM can connect to 24 FECs at most
        end_flag_num[j] = 1;  // 1*24 "one" matrix
    }

    // cout << Source_ID_encode_num[0][31] << " " << card_info_num[0][23] << " " << end_flag_num[0][23] << endl;

    int Source_ID_encode_num[32] = {0}; // 1*32 zero matrix

    int source_ID = 0;
    int Card_num = 0;
    int Chip_num = 0;
    int Chn_num = 0;
    int Data_num = 0;
    // cout << "mark 1\n";

    for (int i = 0; i <= (file_data - 1024); i++)
    {
        // cout << "mark 3\n";
        // in order to avoid the lack of a full data packet near the file end
        // cout << hex << Packet[i] << " " << START_OF_PACKET << " " << bitand(Packet[i + 1], take_SOE_bit) << " \n";
        if (Packet[i] == START_OF_PACKET &&
            ((current_position == 0 && ((Packet[i + 1] & take_SOE_bit) != 0)) || i == current_position + current_packet_length))
        // 不为包头，且，（（当前位置为零且某某不为零）或i=某某）
        {
            // cout << "mark 2\n";
            current_position = i;
            current_packet_length = (Packet[i + 1] & take_Packet_Size_bits) / 2 + 4;
            // cout << "current_position : " << dec << current_position << endl;
            // cout << "current_packet_length : " << current_packet_length << endl;

            // packet header
            if ((Packet[i + 1] & take_SOE_bit) != 0)
            {
                // cout << dec
                //  << Packet[i + 2] << " " << take_SOURCE_ID_bits << " ";
                //  << (Packet[i + 2] & take_SOURCE_ID_bits) << " ";
                //  << bitand(Packet[i + 2], take_SOURCE_ID_bits) << endl;
                source_ID = (Packet[i + 2] & take_SOURCE_ID_bits);
                // timestamp[source_ID + 1] = Packet[i + 3] + Packet[i + 4] * pow(2., 16) + Packet[i + 5] * pow(2., 32); //????
                timestamp[source_ID + 1] = Packet[i + 4] * pow(2., 16) + Packet[i + 5] * pow(2., 32) + Packet[i + 3]; //????
                // cout << dec << Packet[i + 3] << " ";
                // cout << dec << Packet[i + 4] * pow(2., 16) + Packet[i + 5] * pow(2., 32) + Packet[i + 3] << " ";
                // cout << dec << timestamp[source_ID + 1] << " \n";
                // cout << Packet[i + 4] * pow(2., 16) + Packet[i + 5] * pow(2., 32) + Packet[i + 3] << endl;

                // cout << dec << Packet[i + 3] << " ";
                // cout << dec << (Packet[i + 4] << 16) << " " << (Packet[i + 5] << 32) << " ";
                // cout << (Packet[i + 4] << 16) + (Packet[i + 5] << 32) + Packet[i + 3] << endl;

                event_ID[source_ID + 1] = Packet[i + 6] + (Packet[i + 7] << 16);
                // cout << event_ID[source_ID + 1] << endl;
                // cout << dec << source_ID << " " << timestamp[source_ID + 1] << " " << event_ID[source_ID + 1] << endl;
                if (Source_ID_encode_num[source_ID + 1] == 0)
                {
                    Source_ID_encode_num[source_ID + 1] = encode_num;
                    encode_num = encode_num + 1;
                    // cout << Source_ID_encode_num[source_ID + 1] << " " << encode_num << "\n";
                }
            }

            if (((Packet[i + 1] & take_SOE_bit) == 0) && ((Packet[i + 1] & take_EOE_bit) == 0))
            {
                current_packet_length = (Packet[i + 1] & take_Packet_Size_bits) / 2 + 4;
                Card_num = (Packet[i + 2] & take_Card_num_bits) >> 9;

                // cout << "(Packet[i + 21] & take_Chip_num_bits) : " << (Packet[i + 21] & take_Chip_num_bits) << " "
                //      << ((Packet[i + 21] & take_Chip_num_bits) >> 7) << " ";

                Chip_num = (Packet[i + 21] & take_Chip_num_bits) >> 7;
                // cout << Chip_num << endl;
                Chn_num = Packet[i + 2] & take_Chn_num_bits;
                Data_num = 1;

                // cout << current_packet_length << " " << Card_num << " " << Chip_num << " " << Chn_num << " " << Data_num << endl;
                // cout << Chip_num << " ";

                card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][1] = event_ID[Card_num + 1];
                card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][2] = timestamp[Card_num + 1];
                card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][3] = Card_num;
                // 3 for card ID
                // cout << Card_num << endl;
                card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][4] = Chip_num;
                // 4 for chip ID
                card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][5] = Chn_num;
                // 5 for channel ID

                // cout << card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][1] << " "
                //      << card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][2] << " "
                //      << card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][3] << " "
                //      << card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][4] << " "
                //      << card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][5] << " \n";

                for (int j = i + 3; j <= (i + current_packet_length - 4); j++)
                {
                    card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][Data_num + 5] = (Packet[j] & take_ADC_code_bits);
                    Data_num = Data_num + 1;

                    // cout << "mark 5 : " << card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][Data_num + 5] << " "
                    //      << Data_num << "\n";
                    // cout << card_info[Source_ID_encode_num[Card_num + 1]][card_info_num[Source_ID_encode_num[Card_num + 1]]][Data_num + 5] << endl;
                }
                card_info_num[Source_ID_encode_num[Card_num + 1]] = card_info_num[Source_ID_encode_num[Card_num + 1]] + 1;
                // cout << "mark 6 : " << card_info_num[Source_ID_encode_num[Card_num + 1]] << "\n";
            }

            if ((Packet[i + 1] & take_EOE_bit) != 0)
            {
                source_ID = (Packet[i + 2] & take_SOURCE_ID_bits);
                end_flag[(Source_ID_encode_num[source_ID + 1])][(end_flag_num[Source_ID_encode_num[source_ID + 1]])] = card_info_num[Source_ID_encode_num[Card_num + 1]];
                end_flag_num[Source_ID_encode_num[source_ID + 1]] = end_flag_num[Source_ID_encode_num[source_ID + 1]] + 1;

                // cout //<< "mark 7 : " << source_ID << " "
                //     //  << card_info_num[Source_ID_encode_num[Card_num + 1]] << " "
                //     << "source_ID + 1: " << source_ID + 1 << " "
                //     << "Source_ID_encode_num: " << Source_ID_encode_num[source_ID + 1] << " "
                //     << "end_flag_num: " << end_flag_num[Source_ID_encode_num[source_ID + 1]] << " "
                //     << "card_info_num: " << card_info_num[Source_ID_encode_num[Card_num + 1]] << " "
                //     << end_flag[(Source_ID_encode_num[source_ID + 1])][(end_flag_num[Source_ID_encode_num[source_ID + 1]])] << " \n";
                //  << end_flag_num[Source_ID_encode_num[source_ID + 1]] << "\n";
            }
        }
    }
    double baseline_SUM_1, baseline_SUM_2, baseline;
    ofstream xinhao;
    xinhao.open("xinhao.txt");
    for (int i = 1; i < 5; i++){
        for (int j = 0; j < 8192; j++){
            // for (int k = 0; k < 518; k++){
                // cout << i << " " << j << " " << k << " \n";
                // // cout << card_info[i][j][k]  << "\n";
                //     baseline_SUM_1 = accumulate(card_info[i][j]+6, card_info[i][j]+56);
                //     baseline_SUM_2 = accumulate(card_info[i][j]+418, card_info[i][j]+518);
                //     baseline = (baseline_SUM_1 + baseline_SUM_2)/150.;
                if (*max_element(card_info[i][j]+6, card_info[i][j]+518) != 0){
                    baseline_SUM_1 = accumulate(card_info[i][j]+6, card_info[i][j]+56, 0);
                    baseline_SUM_2 = accumulate(card_info[i][j]+418, card_info[i][j]+518, 0);
                    baseline = (baseline_SUM_1 + baseline_SUM_2)/150.;

                    xinhao //<< *max_element(card_info[i][j]+6, card_info[i][j]+518) << " "
                            //    << baseline << " "
                               << card_info[i][j][3] << " " //board, card
                               << card_info[i][j][4]+1 << " " //chip
                               << card_info[i][j][5] << " " //channel
                               << *max_element(card_info[i][j]+6, card_info[i][j]+518) - baseline << " " //adc (peak of wave)
                               << "\n";

                    // cout << *max_element(card_info[i][j]+6, card_info[i][j]+518) << "\n"; 
                }
            // }
        }
    }
    xinhao.close();

    // ofstream signal_B2;
    // signal_B2.open("signal_B2.txt");
    // for (int i = 2; i < 3; i++){
    //     for (int j = 0; j < 8192; j++){
    //             if (*max_element(card_info[i][j]+6, card_info[i][j]+518) != 0){
    //                 baseline_SUM_1 = accumulate(card_info[i][j]+6, card_info[i][j]+56, 0);
    //                 baseline_SUM_2 = accumulate(card_info[i][j]+418, card_info[i][j]+518, 0);
    //                 baseline = (baseline_SUM_1 + baseline_SUM_2)/150.;
    //                 signal_B2 << *max_element(card_info[i][j]+6, card_info[i][j]+518) - baseline << " "
    //                            << "\n";
    //             }
    //     }
    // }
    // signal_B2.close();

    // ofstream signal_B3;
    // signal_B3.open("signal_B3.txt");
    // for (int i = 3; i < 4; i++){
    //     for (int j = 0; j < 8192; j++){
    //             if (*max_element(card_info[i][j]+6, card_info[i][j]+518) != 0){
    //                 baseline_SUM_1 = accumulate(card_info[i][j]+6, card_info[i][j]+56, 0);
    //                 baseline_SUM_2 = accumulate(card_info[i][j]+418, card_info[i][j]+518, 0);
    //                 baseline = (baseline_SUM_1 + baseline_SUM_2)/150.;
    //                 signal_B3 << *max_element(card_info[i][j]+6, card_info[i][j]+518) - baseline << " "
    //                            << "\n";
    //             }
    //     }
    // }
    // signal_B3.close();

    // ofstream signal_B4;
    // signal_B4.open("signal_B4.txt");
    // for (int i = 4; i < 5; i++){
    //     for (int j = 0; j < 8192; j++){
    //             if (*max_element(card_info[i][j]+6, card_info[i][j]+518) != 0){
    //                 baseline_SUM_1 = accumulate(card_info[i][j]+6, card_info[i][j]+56, 0);
    //                 baseline_SUM_2 = accumulate(card_info[i][j]+418, card_info[i][j]+518, 0);
    //                 baseline = (baseline_SUM_1 + baseline_SUM_2)/150.;
    //                 signal_B4 << *max_element(card_info[i][j]+6, card_info[i][j]+518) - baseline << " "
    //                            << "\n";
    //             }
    //     }
    // }
    // signal_B4.close();

    // for (int i = 0; i < 2169; i++)
    // {
    //     // for (int j = 0; j < 8192; j++)
    //     // {
    //     //     for (int k = 1; k <= 8192; k++)
    //     //     {
    //             // cout << k << " " << card_info[1][k][3] << " \n";
    //             ofstream xinhao;
    //             xinhao.open("xinhao.txt", ofstream::app);
    //             xinhao
    //                         // << card_info[1][i][0] << " "
    //                         // << card_info[1][i][1] << " "
    //                         // << card_info[4][i][2] << " "
    //                         << card_info[4][i][3] << " "
    //                         << card_info[4][i][4] << " "
    //                         << card_info[4][i][5] << " "
    //                         // << card_info[1][i][6] << " "
    //                         // << card_info[1][i][7] << " "
    //                         // << card_info[1][i][8] << " "
    //                         // << card_info[1][i][9] << " "
    //                         // << card_info[1][i][10] << " "
    //                         // << card_info[1][i][511] << " "
    //                         // << card_info[1][i][512] << " "
    //                         << "\t" << endl;
    //             xinhao.close();
    //     //     }
    //     // }
    // }

    // for (int k = 0; k <= 10; k++)
    // {
    //     cout << end_flag_num[k] << endl;
    // }

    // FEC data draw
    // int test_board_ID = 0;
    // int test_board_flag = 0;
    // // cout << "Please input test board ID : ";
    // // cin >> test_board_ID;
    // test_board_ID = board_ID;

    // if (test_board_ID == 1)
    // {
    //     test_board_flag = 1;
    // }
    // else if (test_board_ID == 2)
    // {
    //     test_board_flag = 3;
    // }
    // else if (test_board_ID == 3)
    // {
    //     test_board_flag = 4;
    // }
    // else if (test_board_ID == 4)
    // {
    //     test_board_flag = 2;
    // }
     
        // ofstream xinhao;
        // xinhao.open("xinhao.txt");
        // for (int k = 0; k < 600; k++)
        // {
            // for (int j = 0; j < 8192; j++)
            // {
                // for (int i = 1; i <= 8192; i++)
                // {
            // cout << k << " " << card_info[1][1][k] << " \n";
            // xinhao
            //     << card_info[1][i][6] << " "
            //     << card_info[1][i][7] << " "
            //     << card_info[1][i][8] << "; "
                // << "\t" << endl;
                // }
        //     // }
        // }
        // xinhao.close();
    //2021.10.27 试图把card_info[][][]分割成一个四维数组->似不可行，尝试先把每一channel都输出到txt

    // ofstream sig_data1;
    // sig_data1.open("sig_data1.dat");

    //     for(int i = 1; i <= 1; i++){
    //         for (int j = 1; j <= 8192; j++){
    //             // sig_data1 <<  card_info[i][j][3] << " "
    //             //           <<  card_info[i][j][4] << " "
    //             //           <<  card_info[i][j][5] << " \t\n";
    //             for(int k = 1; k <= 512; k++){
    //                 sig_1[i][j][k] = card_info[i][j][k+5];
    //             }
    //         }
    //     }
    // sig_data1.close();
    // ofstream sig_data;
    // sig_data.open("sig_data.txt");
    // for (int i = 1; i <= 8192; i++){
    // //     // cout << b[i] << " " << c[i] << " " << ch[i] << endl;
    //     sig_data << b[i] << " "
    //            << c[i] << " "
    //            << ch[i] << " "
    // //            << "\t"
    //            << endl;
    // }
    // sig_data.close();
    // double ch[4][4][64][8192] = {0.}; //4 boards * 4 cards * 64 channels * 512 bins
    // double sig[512] = {0.};


    /*
    double x[512] = {0};
    double y[30][512] = {0};
    // double RMSA[272][6] = {0};
    // double RMS[272] = {0};
    // double baselineA[272][6] = {0};
    // double baseline[272] = {0};
    // double Threshold[272] = {0}; //272*1 matrix

    for (int i = 0; i < 512; i++)
    {
        x[i] = 6 + i;
        // cout << "x" << dec << i << " " << x[i] << endl;
        // fill x matrix from 6 to 517
    }

    for (int k = 1; k <= 6; k++)
    {
        for (int i = end_flag[test_board_flag][k]; i <= end_flag[test_board_flag][k + 1]; i++)
        {
            // cout << end_flag[1][k] << "-" << i << "-" <<  end_flag[1][k + 1] << " ";

            for (int chip_num = 0; chip_num <= 3; chip_num++)
            {
                for (int chn_num = 0; chn_num <= 67; chn_num++)
                {
                    // cout << card_info[1][i][3] << " " << card_info[1][i][4] << " " << card_info[1][i][5] << endl;
                    // cout << test_board_ID << " " << chip_num << " " << chn_num << endl;

                    if (card_info[test_board_flag][i][3] == test_board_ID &&
                        card_info[test_board_flag][i][4] == chip_num &&
                        card_info[test_board_flag][i][5] == chn_num)
                    {
                        // cout << "s\n";chip_num
                        // cout << test_board_ID << " " << chip_num << " " << chn_num << endl;
                        // cout << "I am HERE.\n";
                        for (int j = 6; j <= 517; j++)
                        {
                            y[i - end_flag[test_board_flag][1]][j - 5] = card_info[test_board_flag][i][j];
                            // cout << "mark 8: " << card_info[1][i][j] << " " << y[j - 5] << endl;
                        }
                        // baselineA[chip_num * 68 + chn_num + 1][k] = mean(y, sizeof(y) / sizeof(double));
                        // RMSA[chip_num * 68 + chn_num + 1][k] = stddev(y, sizeof(y) / sizeof(double));

                        // cout << i << " ";

                        // cout << "mark 9-1 : " << RMSA[chip_num * 68 + chn_num + 1][k] << endl;

                        // cout << y[1] << endl;
                        // cout << baselineA[chip_num * 68 + chn_num + 1][k];
                        // cout << sizeof(y) / sizeof(double) << endl;
                        // cout << mean(y, sizeof(y) / sizeof(double)) << " " << stddev(y, sizeof(y) / sizeof(double)) << endl;
                        // cout << " " << RMSA[chip_num * 68 + chn_num + 1][k] << endl;
                        //趋势一致,具体数值有出入
                    }
                }
            }
        }
    }
    */
    // for (int i = 1; i <= 272; i++)
    // {
    //     for (int j = 1; j <= 6; j++)
    //     {
    //         // cout << "mark 9 : " << RMSA[i][j] << endl;

    //         RMS[i] = mean(RMSA[i], sizeof(RMSA[i]) / sizeof(double));
    //         // cout << "mark 10 : " << RMS[i] << endl;
    //         baseline[i] = mean(baselineA[i], sizeof(baselineA[i]) / sizeof(double));
    //         Threshold[i] = baseline[i] + 7 * RMS[i];
    //     }
    // }

    // TCanvas *c1 = new TCanvas("c1", "c1", 1200, 1000);
    // TCanvas *c2 = new TCanvas("c2", "c2", 600, 500);

    // int test_channel_ID = channel_ID; 
    // c2->cd();
    // // f1
    // int n0 = sizeof(y[30]) / sizeof(double);
    // TGraph *gr0 = new TGraph(n0, x - 5, y[test_channel_ID]);
    // gr0->SetTitle("Wave");
    // gr0->GetXaxis()->SetTitle("cell number");
    // gr0->GetXaxis()->CenterTitle();
    // gr0->GetYaxis()->SetTitle("ADC code");
    // gr0->GetYaxis()->CenterTitle();
    // gr0->Draw("");
    // c2->SaveAs("c2.png");

    /*
        // c1->Divide(2, 2);

        // //f2
        // c1->cd(1);
        // double a1[272] = {0};
        // for (int i = 1; i <= 272; i++)
        // {
        //     a1[i] = i;
        //     // fill x matrix from 1 to 272
        // }
        // int n1 = sizeof(a1) / sizeof(double);

        // TGraph *gr1 = new TGraph(n1, a1, RMS);
        // gr1->SetTitle("noise level");
        // gr1->GetXaxis()->SetTitle("channel number");
        // gr1->GetXaxis()->CenterTitle();
        // gr1->GetYaxis()->SetTitle("RMA/ADC code");
        // gr1->GetYaxis()->CenterTitle();
        // gr1->SetFillColor(1);
        // gr1->Draw("AB");

        // //f3
        // c1->cd(2);
        // double a2[512] = {0};
        // for (int i = 1; i <= 512; i++)
        // {
        //     a2[i] = i;
        //     // cout << "x" << dec << i << " " << x[i] << endl;
        //     // fill x matrix from 1 to 512
        // }
        // double b[512] = {0};
        // for (int j = 6; j <= 517; j++)
        // {
        //     b[j] = card_info[1][j][1];
        // }
        // int n2 = sizeof(a2) / sizeof(double);

        // TGraph *gr2 = new TGraph(n2, a2, b);
        // gr2->SetTitle("Wave");
        // gr2->Draw("");

        // //f4
        // c1->cd(3);
        // TH1 *h2 = new TH1F("h2", "h2 title", 1024, 0, 4096);
        // double d[272] = {0};
        // double c[272] = {0};
        // for (int i = 0; i <= 272; i++)
        // {
        //     d[i] = i;
        // }
        // double len_data = sizeof(card_info) / sizeof(card_info[0][5]);
        // len_data = 4;
        // // cout << "mark 11 : " << len_data << endl;
        // for (int i = 1; i <= len_data; i++)
        // {
        //     c[i] = card_info[1][i][5] + 68 * card_info[1][i][4];
        //     h2->Fill(c[i]);
        //     // cout << "mark 12" << endl;

        // }
        // h2->Draw();
        // // int n3 = sizeof(d) / sizeof(double);

        // // TGraph *gr3 = new TGraph(n3, d, c);
        // // gr3->SetTitle("");
        // h2->GetXaxis()->SetTitle("hit channel");
        // h2->GetXaxis()->CenterTitle();
        // h2->GetYaxis()->SetTitle("count");
        // h2->GetYaxis()->CenterTitle();
        // gr3->SetFillColor(1);
        // gr3->Draw("AB");

        //f5
        // double e[272] = {0};
        // for(int i = 1; i <= len_data; i++){
        //     if(card_info[1][i][5] == 36 && card_info[1][i][4] == 3){
        //         j = j + 1;
        //         for(int k = 5; k <= 517; k++)
        //         {
        //             if(card_info[1][i][k])
        //         }
        //         e[j] = *max_element(card_info[1][i][]);
        //     }
        // }

    */
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
int hex2dec(string a)
{
    int n = 0;
    int x = 4;
    for (int i = 0; i < x; i++)
        if (a[i] <= '9')
            n += (a[i] - '0') * pow(16, (x - i - 1));
        else
            n += (a[i] - 'a' + 10) * pow(16, (x - i - 1));
    // cout << "十六进制数" << a << "转化后的十进制数：" << n << endl;
    return n;
}

double mean(double *data, double n)
{
    double sum = 0;
    for (int j = 0; j < n; j++)
    {
        sum = sum + data[j];
    }
    return sum / n;
}

double stddev(double *data, double n)
{
    double sum = 0.;
    double mea = 0.;
    double del = 0.;
    // double zero_c = 0;
    // for (int z = 0; z < n; z++){
    //     if(data[z] == 0){
    //         zero_c = zero_c + 1;
    //     }
    // }

    for (int j = 0; j < n; j++)
    {
        sum = sum + data[j];
    }
    mea = sum / n;
    // cout << "mea: " << mea << endl;

    for (int k = 0; k < n; k++)
    {
        del = del + pow(data[k] - mea, 2);
        // cout << "data[k] - mea: " << pow(data[k] - mea, 2) << endl;
    }
    // cout << "del: " << del/n << endl;
    return sqrt(del / n);
}

short high_low_exchange(short data1)
{
    short high = (data1 & 0xff00) >> 8;
    short low = data1 & 0xff;

    short data;
    data = (low << 8) | high;
    return data;
}