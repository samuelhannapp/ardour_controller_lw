#include "main.hpp"
#include <sstream>
#include <cstdlib>
#include <clocale>
#include <stdio.h>

#ifdef _WIN64
#include <SDKDDKVer.h>
#define _WINSOCKAPI_
#include <Windows.h>
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif
#ifdef __linux__
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

#define WIDGET_WIDTH 150
#define WIDGET_HEIGHT 34
enum
{
    ID_START_ARDOUR_CONTROLLER = 2, ID_OTHER, ID_THIRD, ID_PLUGIN
};

bool MyApp::OnInit()
{
	MyFrame* frame = new MyFrame();
	frame->Show();
	return true;
}

MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "Base Controller (Mackie Control) Display", wxDefaultPosition, wxSize(800, 400))
{

    menuConnections = new wxMenu;
    menuConnections->Append(ID_START_ARDOUR_CONTROLLER, "&Start Ardour Controller", "setup ip address for controller");
    menuConnections->Append(ID_OTHER, "&other...\tCtrl+H", "adjust display size to controller");
    menuConnections->AppendSeparator();
    menuConnections->Append(wxID_EXIT);

    menuDisplayFormat = new wxMenu;
    menuDisplayFormat->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuConnections, "Settings");
    menuBar->Append(menuDisplayFormat, "about");

    SetMenuBar(menuBar);

    //CreateStatusBar();
    //SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnStartArdourController, this, ID_START_ARDOUR_CONTROLLER);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_CLOSE_WINDOW, &MyFrame::OnClose, this);


    CreateStatusBar();
    button_sizer = new wxBoxSizer(wxHORIZONTAL);
    add_controller_button = new wxButton(this, wxID_ANY, " Add Controller");
    add_controller_button->Bind(wxEVT_BUTTON, &MyFrame::OnAddController, this);
    start_ardour_controller_button = new wxButton(this, wxID_ANY, " Start controller");
    start_ardour_controller_button->Bind(wxEVT_BUTTON, &MyFrame::OnStartController, this);

    button_sizer->Add(add_controller_button);
    button_sizer->Add(start_ardour_controller_button);

    for (int i = 0; i < 8; i++)
        header_panels.push_back(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(WIDGET_WIDTH, WIDGET_HEIGHT), wxBORDER_RAISED));

    header_lables.push_back(new wxStaticText(header_panels.at(0), wxID_ANY, "controller"));
    header_lables.push_back(new wxStaticText(header_panels.at(1), wxID_ANY, "Ardour IP"));
    header_lables.push_back(new wxStaticText(header_panels.at(2), wxID_ANY, "Port from Ardour"));
    header_lables.push_back(new wxStaticText(header_panels.at(3), wxID_ANY, "Port to Ardour"));
    header_lables.push_back(new wxStaticText(header_panels.at(4), wxID_ANY, "Display enable"));
    header_lables.push_back(new wxStaticText(header_panels.at(5), wxID_ANY, "Display IP"));
    header_lables.push_back(new wxStaticText(header_panels.at(6), wxID_ANY, "Port from display"));
    header_lables.push_back(new wxStaticText(header_panels.at(7), wxID_ANY, "Port to display"));

    header_lables.at(0)->SetToolTip("choose hardware or gui controller");
    header_lables.at(1)->SetToolTip("If controller is on the same computer, than use Local Host Address (127.0.0.1)\nIf Controller is on another computer select the IP address of the Computer where Ardour is running");
    header_lables.at(2)->SetToolTip("Udp Port to receive data from Arduor\n Only needs to be adjusted if the UDP Ports conflict with something else");
    header_lables.at(3)->SetToolTip("Udp Port to send data to Arduor\n Only needs to be adjusted if the UDP Ports conflict with something else");
    header_lables.at(4)->SetToolTip("You can use a display for a base controller (Mackie Control) or also for others if available");
    header_lables.at(5)->SetToolTip("If the display is on the same computer, than use Local Host Address (127.0.0.1)\nIf Controller is on another computer select the IP address of the Computer where the Display is running");
    header_lables.at(6)->SetToolTip("Udp Port to receive data from Display\n Only needs to be adjusted if the UDP Ports conflict with something else");
    header_lables.at(7)->SetToolTip("Udp Port to send data to Display\n Only needs to be adjusted if the UDP Ports conflict with something else");

    header_panels.at(0)->SetToolTip("choose hardware or gui controller");
    header_panels.at(1)->SetToolTip("If controller is on the same computer, than use Local Host Address (127.0.0.1)\nIf Controller is on another computer select the IP address of the Computer where Ardour is running");
    header_panels.at(2)->SetToolTip("Udp Port to receive data from Arduor\n Only needs to be adjusted if the UDP Ports conflict with something else");
    header_panels.at(3)->SetToolTip("Udp Port to send data to Arduor\n Only needs to be adjusted if the UDP Ports conflict with something else");
    header_panels.at(4)->SetToolTip("You can use a display for a base controller (Mackie Control) or also for others if available");
    header_panels.at(5)->SetToolTip("If the display is on the same computer, than use Local Host Address (127.0.0.1)\nIf Controller is on another computer select the IP address of the Computer where the Display is running");
    header_panels.at(6)->SetToolTip("Udp Port to receive data from Display\n Only needs to be adjusted if the UDP Ports conflict with something else");
    header_panels.at(7)->SetToolTip("Udp Port to send data to Display\n Only needs to be adjusted if the UDP Ports conflict with something else");

    header_sizer = new wxBoxSizer(wxHORIZONTAL);
    header_sizer->Add(header_panels.at(0));
	header_sizer->Add(header_panels.at(1));
	header_sizer->Add(header_panels.at(2));
	header_sizer->Add(header_panels.at(3));
	header_sizer->Add(header_panels.at(4));
	header_sizer->Add(header_panels.at(5));
    header_sizer->Add(header_panels.at(6));
    header_sizer->Add(header_panels.at(7));

    main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(button_sizer);
    main_sizer->Add(header_sizer);

    this->gui_controllers.push_back(plugin_routing_customizer_string);
    this->gui_controllers.push_back(gp_controller_string);
    this->gui_controllers.push_back(sp_controller_string);
    this->gui_controllers.push_back(mackie_controller_gui_version_string);

    PrintMidiDevices();

    this->SetSizerAndFit(main_sizer);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    #ifdef __linux__
    for(int pid : this->pids)
		kill(pid, SIGKILL);
    #endif
    #ifdef _WIN64
    #endif
    Close(true);
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    #ifdef __linux__
    for(int pid : this->pids)
		kill(pid, SIGTERM);
    #endif
    event.Skip();
}


void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnStartArdourController(wxCommandEvent& event)
{
    wxMessageBox("Ardour Controller Started",
        "Ardour Controller", wxOK | wxICON_INFORMATION);
    event.Skip();
    return;
}

#define UDP_START_PORT 9

void MyFrame::OnAddController(wxCommandEvent& event)
{
    wxChoice* choice = new wxChoice(this, wxID_ANY);
    choice->SetToolTip("chose mackie control or gui version");
    choice->SetMinSize(wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    wxArrayString choices;
    for (std::string controller_name : this->midi_in_devices)
        choices.Add(controller_name);
    for (std::string controller_name : this->gui_controllers)
        choices.Add(controller_name);
    choice->Set(choices);
    this->controllers.push_back(choice);

    wxTextCtrl* ardour_ip_address = new wxTextCtrl(this, wxID_ANY, "127.0.0.1", wxDefaultPosition, wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    this->ardour_network_address.push_back(ardour_ip_address);
    
    int port_nr = UDP_START_PORT + (this->controllers.size() * 3);
    wxSpinCtrl* spin_control_from_ardour = new wxSpinCtrl(this, wxID_ANY, std::to_string(port_nr));
    spin_control_from_ardour->SetMinSize(wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    receive_port_from_ardour.push_back(spin_control_from_ardour);

    wxSpinCtrl* spin_control_to_ardour = new wxSpinCtrl(this, wxID_ANY, std::to_string(send_port_to_ardour_udp_nr));
    spin_control_to_ardour->SetMinSize(wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    spin_control_to_ardour->SetRange(3819, 3819);
    send_port_to_ardour.push_back(spin_control_to_ardour);

    wxCheckBox* check_box = new wxCheckBox(this, wxID_ANY, "Display", wxDefaultPosition, wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    this->display_enable_check_box.push_back(check_box);
    check_box->SetValue(true);

    wxTextCtrl* display_ip_address = new wxTextCtrl(this, wxID_ANY, "127.0.0.1", wxDefaultPosition, wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    this->display_network_address.push_back(display_ip_address);

    wxSpinCtrl* spin_control_send_port_to_display = new wxSpinCtrl(this, wxID_ANY, std::to_string(port_nr + 1));
    spin_control_send_port_to_display->SetMinSize(wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    send_port_to_display.push_back(spin_control_send_port_to_display);

    wxSpinCtrl* spin_control_receive_port_from_display = new wxSpinCtrl(this, wxID_ANY, std::to_string(port_nr + 2));
    spin_control_receive_port_from_display->SetMinSize(wxSize(WIDGET_WIDTH, WIDGET_HEIGHT));
    receive_port_from_display.push_back(spin_control_receive_port_from_display);

    this->controller_layout.push_back(new wxBoxSizer(wxHORIZONTAL));
    this->controller_layout.back()->Add(choice);
    this->controller_layout.back()->Add(ardour_ip_address);
    this->controller_layout.back()->Add(spin_control_from_ardour);
    this->controller_layout.back()->Add(spin_control_to_ardour);
    this->controller_layout.back()->Add(check_box);
    this->controller_layout.back()->Add(display_ip_address);
    this->controller_layout.back()->Add(spin_control_send_port_to_display);
    this->controller_layout.back()->Add(spin_control_receive_port_from_display);

    this->main_sizer->Add(this->controller_layout.back());
    this->SetSizerAndFit(this->main_sizer);

    event.Skip();
}
#ifdef _WIN64
int MyFrame::get_midi_in(std::string controller_name)
{
    for (int i = 0; i < this->midi_in_devices.size(); i++) {
        if (!this->midi_in_devices.at(i).compare(controller_name))
            return i;
    }
    return -1;
}

int MyFrame::get_midi_out(std::string controller_name)
{
    for (int i = 0; i < this->midi_out_devices.size(); i++) {
        if (!this->midi_out_devices.at(i).compare(controller_name))
            return i;
    }
    return -1;
}
#endif

#ifdef __linux__ 
int MyFrame::get_midi_in(std::string controller_name)
{
    for (int i = 0; i < this->midi_in_devices.size(); i++) {
        if (!this->midi_in_devices.at(i).compare(controller_name))
            return this->card[i]; 
    }
    return -1;
}

int MyFrame::get_midi_out(std::string controller_name)
{
    for (int i = 0; i < this->midi_out_devices.size(); i++) {
        if (!this->midi_out_devices.at(i).compare(controller_name))
            return this->card[i];
    }
    return -1;
}
#endif

void MyFrame::start_plugin_routing_customizer()
{
    #ifdef _WIN64
        std::string plugin_routing_customizer_exe_path = ("plugin_router.exe");
    #endif
    #ifdef __linux__
        std::string plugin_routing_customizer_exe_path = ("./plugin_routing_customizer");
    #endif
    start_process(plugin_routing_customizer_exe_path);
}

void MyFrame::start_base_controller_udp(int index)
{
    #ifdef _WIN64
    std::string base_controller_exe_path = ("base_controller.exe");
    std::string gui_controller_exe_path = ("gui_controller.exe");
    std::string display_exe_path = ("mackie_display.exe");
    #endif

    #ifdef __linux__ 
    std::string base_controller_exe_path = ("./osc_controller_UDP");
    std::string gui_controller_exe_path = ("./base_controller_gui_version");
    std::string display_exe_path = ("./base_controller_display");
    #endif

    int ardour_controller_in_port = receive_port_from_ardour.at(index)->GetValue();
	int ardour_controller_out_port = send_port_to_ardour.at(index)->GetValue();

    std::string controller_name = this->controllers.at(index)->GetStringSelection().ToStdString();

	//int midi_in_nr = this->get_midi_in(controller_name);
	//int midi_out_nr = this->get_midi_out(controller_name);

	int display_in = this->receive_port_from_display.at(index)->GetValue();
	int display_out = this->send_port_to_display.at(index)->GetValue();

    //I just use now an offset from the display udp port's, let's see wether this is a good Idea...
    int midi_in_nr = display_in + 50;
    int midi_out_nr = display_out + 50;

	bool display_enabled = this->display_enable_check_box.at(index)->GetValue();

	std::vector<std::string> base_controller_arguments;
	base_controller_arguments.push_back(std::to_string(ardour_controller_in_port));
	base_controller_arguments.push_back(std::to_string(ardour_controller_out_port));
	base_controller_arguments.push_back(std::to_string(midi_in_nr));
	base_controller_arguments.push_back(std::to_string(midi_out_nr));
	base_controller_arguments.push_back(std::to_string(display_in));
	base_controller_arguments.push_back(std::to_string(display_out));

    std::vector<std::string> gui_controller_arguments;
    gui_controller_arguments.push_back(std::to_string(midi_out_nr));
    gui_controller_arguments.push_back(std::to_string(midi_in_nr));

	std::vector<std::string> display_arguments;
	display_arguments.push_back(std::to_string(display_out));
	display_arguments.push_back(std::to_string(display_in));

	start_process(base_controller_exe_path, base_controller_arguments);
    start_process(gui_controller_exe_path, gui_controller_arguments);
	if(display_enabled)
		start_process(display_exe_path, display_arguments);
}

void MyFrame::start_base_controller_midi(int index)
{
    #ifdef _WIN64
    std::string base_controller_exe_path = ("base_controller_midi_version.exe");
    std::string display_exe_path = ("mackie_display.exe");
    #endif

    #ifdef __linux__

    std::string base_controller_exe_path = ("./osc_controller_MIDI");
    std::string display_exe_path = ("./base_controller_display");
    #endif

    int ardour_controller_in_port = receive_port_from_ardour.at(index)->GetValue();
	int ardour_controller_out_port = send_port_to_ardour.at(index)->GetValue();

    std::string controller_name = this->controllers.at(index)->GetStringSelection().ToStdString();

	int midi_in_nr = this->get_midi_in(controller_name);
	int midi_out_nr = this->get_midi_out(controller_name);

	int display_in = this->receive_port_from_display.at(index)->GetValue();
	int display_out = this->send_port_to_display.at(index)->GetValue();

	bool display_enabled = this->display_enable_check_box.at(index)->GetValue();

	std::vector<std::string> base_controller_arguments;
	base_controller_arguments.push_back(std::to_string(ardour_controller_in_port));
	base_controller_arguments.push_back(std::to_string(ardour_controller_out_port));
	base_controller_arguments.push_back(std::to_string(midi_in_nr));
	base_controller_arguments.push_back(std::to_string(midi_out_nr));
	base_controller_arguments.push_back(std::to_string(display_in));
	base_controller_arguments.push_back(std::to_string(display_out));

	std::vector<std::string> display_arguments;
	display_arguments.push_back(std::to_string(display_out));
	display_arguments.push_back(std::to_string(display_in));

	start_process(base_controller_exe_path, base_controller_arguments);
	if(display_enabled)
		start_process(display_exe_path, display_arguments);
}

void MyFrame::start_gui_controller(int index)
{
    std::string controller_name = this->controllers.at(index)->GetStringSelection().ToStdString();
    if(!controller_name.compare(this->gp_controller_string))
        start_gp_controller(index);
    if(!controller_name.compare(this->sp_controller_string))
        start_sp_controller(index);
    if (!controller_name.compare(this->plugin_routing_customizer_string))
        start_plugin_routing_customizer();
    if (!controller_name.compare(this->mackie_controller_gui_version_string))
        start_base_controller_udp(index);
}

void MyFrame::start_gp_controller(int index)
{
    #ifdef _WIN64
    std::string gp_controller_gui_version_exe_path = ("gui_gp_controller.exe");
    #endif
    #ifdef __linux__
    std::string gp_controller_gui_version_exe_path = ("./gp_controller_gui_version");
    #endif
    int ardour_controller_in_port = receive_port_from_ardour.at(index)->GetValue();
	int ardour_controller_out_port = send_port_to_ardour.at(index)->GetValue();

    std::vector<std::string> gp_controller_arguments;
	gp_controller_arguments.push_back(std::to_string(ardour_controller_in_port));
	gp_controller_arguments.push_back(std::to_string(ardour_controller_out_port));
    start_process(gp_controller_gui_version_exe_path, gp_controller_arguments);

    return;
}

void MyFrame::start_sp_controller(int index)
{
    #ifdef _WIN64
    std::string sp_controller_gui_version_exe_path = ("gui_sp_controller.exe");
    #endif
    #ifdef __linux__
    std::string sp_controller_gui_version_exe_path = ("./sp_controller_gui_version");
    #endif
    int ardour_controller_in_port = receive_port_from_ardour.at(index)->GetValue();
	int ardour_controller_out_port = send_port_to_ardour.at(index)->GetValue();

    std::vector<std::string> sp_controller_arguments;
	sp_controller_arguments.push_back(std::to_string(ardour_controller_in_port));
	sp_controller_arguments.push_back(std::to_string(ardour_controller_out_port));
    start_process(sp_controller_gui_version_exe_path, sp_controller_arguments);

    return;
}

bool MyFrame::is_controller_gui_controller(std::string name)
{
    for (std::string controller_name : this->gui_controllers)
        if (!name.compare(controller_name))
            return true;
    return false;
}

void MyFrame::OnStartController(wxCommandEvent& event)
{
    int how_many_controllers = this->controllers.size();

    bool controller_exist = false;

    for (int i = 0; i < how_many_controllers; i++) {

        controller_exist = true;

         std::string controller_name = this->controllers.at(i)->GetStringSelection().ToStdString();

         if (controller_name.empty()) {
             wxMessageBox("You have to select a controller first",
                 "No controller selected", wxOK | wxICON_INFORMATION);
             return;
         }

        if(is_controller_gui_controller(controller_name)){
            start_gui_controller(i);
            continue;
        }
        
        start_base_controller_midi(i);
    }

    if(controller_exist == false)
        wxMessageBox("You have to add a controller first", "No controller added", wxOK | wxICON_INFORMATION);

    event.Skip();
}

#ifdef _WIN64 
void MyFrame::PrintMidiDevices()
{
    UINT nMidiDeviceNum;
    MIDIINCAPS caps_in;
    MIDIOUTCAPS caps_out;

    nMidiDeviceNum = midiInGetNumDevs();
    if (nMidiDeviceNum == 0) {
        fprintf(stderr, "midiInGetNumDevs() return 0...");
        wxMessageBox("Ardour Controller Started",
        "Ardour Controller", wxOK | wxICON_INFORMATION);
        return;
    }

    printf("== PrintMidiDevices() == \n");
    for (unsigned int i = 0; i < nMidiDeviceNum; ++i) {
        midiInGetDevCaps(i, &caps_in, sizeof(MIDIINCAPS));
        std::wstring ws(caps_in.szPname);
        std::string str(ws.begin(), ws.end());
        this->midi_in_devices.push_back(str);
    }

    nMidiDeviceNum = midiOutGetNumDevs();

    for (unsigned int i = 0; i < nMidiDeviceNum; ++i) {
        midiOutGetDevCaps(i, &caps_out, sizeof(MIDIINCAPS));
        std::wstring ws(caps_out.szPname);
        std::string str(ws.begin(), ws.end());
        this->midi_out_devices.push_back(str);
    }
}
#endif
#ifdef __linux__
void MyFrame::PrintMidiDevices() {
    snd_seq_t* seq_handle;
    int err;
    err = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
    if (err < 0) {
        printf("There are no midi devices connected to this computer...\n");
    }
    snd_seq_set_client_name(seq_handle, "My Client");

    int count = 0;
    int status;
    snd_seq_client_info_t* info;

    snd_seq_client_info_alloca(&info);

    status = snd_seq_get_any_client_info(seq_handle, 0, info);
    while (status >= 0) {
        count += 1;
        int id = snd_seq_client_info_get_client(info);
        char const* name = snd_seq_client_info_get_name(info);
        snd_seq_client_type_t device_type = snd_seq_client_info_get_type(info);
        int card = snd_seq_client_info_get_card(info);
        std::string str(name);
        //QString in(caps.szPname);
        if(card != -1){
            this->midi_in_devices.push_back(str);
            this->midi_out_devices.push_back(str);
            this->card.push_back(card);
        }
        int num_ports = snd_seq_client_info_get_num_ports(info);
        printf("Client midi_version: %i, name: �%s� #%i, with %i ports\n", card, name, device_type, num_ports);
        status = snd_seq_query_next_client(seq_handle, info);
    }
}
/*
void MyApp::PrintMidiDevices() {
    int card = -1;

    if (snd_card_next(&card) < 0 || card < 0) {
        printf("No sound cards found\n");
        return;
    }

    while (card >= 0) {
        snd_ctl_t *ctl;
        char name[32];
        name[0] = 'h';
        name[1] = 'w';
        name[2] = ':';

        std::string number = std::to_string(card);

        for(int i = 0; i < number.size(); i++)
            name[i + 3] = number[i];
        
        name[3 + number.size()] = 0;
        name[3 + number.size() + 1] = 0;

        //sprintf(name, "hw:%d", card);


        if (snd_ctl_open(&ctl, name, 0) < 0) {
            snd_card_next(&card);
            continue;
        }

        int device = -1;

        while (1) {
            if (snd_ctl_rawmidi_next_device(ctl, &device) < 0)
                break;

            if (device < 0)
                break;
		    snd_rawmidi_stream_t asdf;	
            for (int stream = 0; stream < 2; stream++) {
                snd_rawmidi_info_t *info;
                snd_rawmidi_info_alloca(&info);

                snd_rawmidi_info_set_device(info, device);
                snd_rawmidi_info_set_stream(info, asdf);
                snd_rawmidi_info_set_subdevice(info, 0);
                int problem = snd_ctl_rawmidi_info(ctl, info);
                if (problem < 0)
                    continue;

                int subs = snd_rawmidi_info_get_subdevices_count(info);

                for (int sub = 0; sub < subs; sub++) {
                    snd_rawmidi_info_set_subdevice(info, sub);

                    if (snd_ctl_rawmidi_info(ctl, info) == 0) {
                        printf("Card %d, Device %d, Subdevice %d\n",
                               card, device, sub);

                        printf("  ID: %s\n",
                               snd_rawmidi_info_get_id(info));
                        printf("  Name: %s\n",
                               snd_rawmidi_info_get_name(info));
                        printf("  Subdevice name: %s\n",
                               snd_rawmidi_info_get_subdevice_name(info));
                    }
                }
            }
        }

        snd_ctl_close(ctl);
        snd_card_next(&card);
    }

    return;
}
    */


#endif

#ifdef __linux__
void MyFrame::start_process(std::string path)
{
    char *args[] = {NULL};
    char executable_cstr[200];
    std::copy(path.begin(), path.end(), executable_cstr); 
    executable_cstr[path.size()] = '\0';

    int pid = fork();
    if(pid == 0)
        execvp(executable_cstr, args);
}

//we maybe have to change the default to std::string and than convert it to wstring...
//this whole wstring nonesense is so annoying...

//we actually could justoscmessage do arguments with no content, and that would fulfill the purpose easier 
//than having two seperate functions...
/*
void MyFrame::start_process(std::string path, std::vector<std::string> arguments)
{

    char executable_cstr[200];
    std::copy(path.begin(), path.end(), executable_cstr); 
    executable_cstr[path.size()] = '\0';

    std::string program_name = path.substr(path.find_last_of('/') + 1, path.size());


    char *args[10];
    args[0] = program_name.data(); 
    for(int i = 0; i < arguments.size(); i++)
        args[i + 1] = arguments.at(i).data();

    args[arguments.size() + 1] = NULL;

    int pid = fork();
    if(pid == 0)
        execvp(executable_cstr, args);
}
*/

//attemt with sudo...
void MyFrame::start_process(std::string path, std::vector<std::string> arguments)
{
    std::string sudo_command("sudo");
    //std::string program_name = path.substr(path.find_last_of('/') + 1, path.size());
    char sudo_command_cstr[200];
    std::copy(sudo_command.begin(), sudo_command.end(), sudo_command_cstr); 
    sudo_command_cstr[sudo_command.size()] = '\0';

    char *args[10];
    args[0] = sudo_command.data(); 
    args[1] = path.data(); 
    for(int i = 0; i < arguments.size(); i++)
        args[i + 2] = arguments.at(i).data();

    args[arguments.size() + 2] = NULL;

    int pid = fork();
    if(pid)
        this->pids.push_back(pid);

    if(pid == 0)
        execvp(sudo_command_cstr, args);
}
#endif

#ifdef _WIN64 

void MyFrame::start_process(std::string path)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    wchar_t temp_array[200];
    int size = 0;
    size = std::mbstowcs(temp_array, path.c_str(), path.size());
    std::wstring std_wstring_path(temp_array, size);

    wchar_t wchar_command[200];    
    int index = 0;
    for (index = 0; index < std_wstring_path.size(); index++)
        wchar_command[index] = std_wstring_path[index];
    wchar_command[index] = '\0';

    // Start the child process. 
    if (!CreateProcess(
        wchar_command,
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }

    // Wait until child process exits.
    //WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    //CloseHandle(pi.hProcess);
    //CloseHandle(pi.hThread);
}
void MyFrame::start_process(std::string path, std::vector<std::string> arguments)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));


    wchar_t temp_array[200];
    int size = 0;
    size = std::mbstowcs(temp_array, path.c_str(), path.size());
    std::wstring std_wstring_path(temp_array, size);
     
    
    std::wstring std_wstring_arguments;
    for (std::string argument : arguments) {
        size = std::mbstowcs(temp_array, argument.c_str(), argument.size());
        std_wstring_arguments.push_back(L' ');
        std_wstring_arguments.append(std::wstring(temp_array, size));
    }


    wchar_t wchar_arguments[200];

    std::wstring input;
    input.append(std_wstring_path);
    input.append(std_wstring_arguments);

    wchar_t wchar_command[200];
    int index = 0;
    for (index = 0; index < input.size(); index++)
        wchar_command[index] = input[index];
    wchar_command[index] = '\0';

    // Start the child process. 
    if (!CreateProcess(
        NULL,
        wchar_command,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }

    // Wait until child process exits.
    //WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    //CloseHandle(pi.hProcess);
    //CloseHandle(pi.hThread);
}
#endif
