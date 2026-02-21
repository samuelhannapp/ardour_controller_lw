#include "main.hpp"
#include <sstream>
#include <cstdlib>
#include <clocale>

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

    CreateStatusBar();
    button_sizer = new wxBoxSizer(wxHORIZONTAL);
    add_controller_button = new wxButton(this, wxID_ANY, " Add Controller");
    add_controller_button->Bind(wxEVT_BUTTON, &MyFrame::OnAddController, this);
    start_ardour_controller_button = new wxButton(this, wxID_ANY, " Start controller");
    start_ardour_controller_button->Bind(wxEVT_BUTTON, &MyFrame::OnStartController, this);

    button_sizer->Add(add_controller_button);
    button_sizer->Add(start_ardour_controller_button);

    for (int i = 0; i < 8; i++)
        header_panels.push_back(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(WIDGET_WIDTH, 20), wxBORDER_RAISED));

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
    Close(true);
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
    choice->SetMinSize(wxSize(WIDGET_WIDTH, 20));
    wxArrayString choices;
    for (std::string controller_name : this->midi_in_devices)
        choices.Add(controller_name);
    for (std::string controller_name : this->gui_controllers)
        choices.Add(controller_name);
    choice->Set(choices);
    this->controllers.push_back(choice);

    wxTextCtrl* ardour_ip_address = new wxTextCtrl(this, wxID_ANY, "127.0.0.1", wxDefaultPosition, wxSize(WIDGET_WIDTH, 20));
    this->ardour_network_address.push_back(ardour_ip_address);
    
    int port_nr = UDP_START_PORT + (this->controllers.size() * 3);
    wxSpinCtrl* spin_control_from_ardour = new wxSpinCtrl(this, wxID_ANY, std::to_string(port_nr));
    spin_control_from_ardour->SetMinSize(wxSize(WIDGET_WIDTH, 20));
    receive_port_from_ardour.push_back(spin_control_from_ardour);

    wxSpinCtrl* spin_control_to_ardour = new wxSpinCtrl(this, wxID_ANY, std::to_string(send_port_to_ardour_udp_nr));
    spin_control_to_ardour->SetMinSize(wxSize(WIDGET_WIDTH, 20));
    spin_control_to_ardour->SetRange(3819, 3819);
    send_port_to_ardour.push_back(spin_control_to_ardour);

    wxCheckBox* check_box = new wxCheckBox(this, wxID_ANY, "Display", wxDefaultPosition, wxSize(WIDGET_WIDTH, 20));
    this->display_enable_check_box.push_back(check_box);
    check_box->SetValue(true);

    wxTextCtrl* display_ip_address = new wxTextCtrl(this, wxID_ANY, "127.0.0.1", wxDefaultPosition, wxSize(WIDGET_WIDTH, 20));
    this->display_network_address.push_back(display_ip_address);

    wxSpinCtrl* spin_control_send_port_to_display = new wxSpinCtrl(this, wxID_ANY, std::to_string(port_nr + 1));
    spin_control_send_port_to_display->SetMinSize(wxSize(WIDGET_WIDTH, 20));
    send_port_to_display.push_back(spin_control_send_port_to_display);

    wxSpinCtrl* spin_control_receive_port_from_display = new wxSpinCtrl(this, wxID_ANY, std::to_string(port_nr + 2));
    spin_control_receive_port_from_display->SetMinSize(wxSize(WIDGET_WIDTH, 20));
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

void MyFrame::start_plugin_routing_customizer()
{
    #ifdef _WIN64
        std::string plugin_routing_customizer_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\plugin_routing_customizer\\plugin_router\\x64\\Debug\\plugin_router.exe");
    #endif
    #ifdef __linux__
        std::wstring plugin_routing_customizer_exe_path = (L"/home/samuel/ardour_controller_lw/plugin_routing_customizer/plugin_routing_customizer.out");
    #endif
    start_process(plugin_routing_customizer_exe_path);
}

void MyFrame::start_base_controller_udp(int index)
{
    std::string base_controller_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\base_controller\\x64\\udp_version\\base_controller.exe");
    std::string gui_controller_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\gui_controller\\x64\\Debug\\gui_controller.exe");
    std::string display_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\display\\mackie_display\\x64\\Debug\\mackie_display.exe");

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
    std::string base_controller_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\base_controller\\x64\\midi_version\\base_controller.exe");
    std::string display_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\display\\mackie_display\\x64\\Debug\\mackie_display.exe");

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
    std::string gp_controller_gui_version_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\gui_gp_controller\\gui_gp_controller\\x64\\Debug\\gui_gp_controller.exe");
    #endif
    #ifdef __linux__
    std::wstring gp_controller_gui_version_exe_path = (L"/home/samuel/ardour_controller_lw/gui_gp_controller/gui_gp_controller/gui_gp_controller/gui_gp_controller.out");
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
    std::string sp_controller_gui_version_exe_path = ("C:\\Users\\Samuel\\Software\\arduor_controller_lw\\gui_sp_controller\\gui_sp_controller\\x64\\Debug\\gui_sp_controller.exe");
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


    std::wstring base_controller_gui_version_exe_path = (L"C:\\Users\\Samuel\\Software\\arduor_controller_lw\\gui_controller\\x64\\Debug\\gui_controller.exe");
    


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
        std::string str(name);
        //QString in(caps.szPname);
        this->midi_in_devices.push_back(str);
        this->midi_out_devices.push_back(str);
        int num_ports = snd_seq_client_info_get_num_ports(info);
        printf("Client �%s� #%i, with %i ports\n", name, id, num_ports);
        status = snd_seq_query_next_client(seq_handle, info);
    }
}
#endif

#ifdef __linux__
void MyFrame::start_process(std::wstring path)
{
   // first we fork the process
    char executable_cstr[200];
    char *args[] = {NULL};
    int size = path.size();
    std::wcstombs(executable_cstr, path.c_str(), path.size());
    executable_cstr[size] = '\0';
    //"/home/samuel/ardour_controller_lw/plugin_routing_customizer/plugin_routing_customizer.out"
    int pid = fork();
    if(pid == 0)
        execvp(executable_cstr, args);
        //execvp(executable_cstr, args);
}

//we maybe have to change the default to std::string and than convert it to wstring...
//this whole wstring nonesense is so annoying...

//we actually could just do arguments with no content, and that would fulfill the purpose easier 
//than having two seperate functions...
void MyFrame::start_process(std::wstring path, std::wstring arguments)
{
    char arguments_cstr[200];
    std::wcstombs(arguments_cstr, arguments.c_str(), arguments.size());
    arguments_cstr[arguments.size()] = '\0'; 
    std::string arguments_std_string(arguments_cstr);
    std::stringstream stream;
    stream.str(arguments_std_string);
    std::vector<std::string> argument_list;
    std::string segment;
    while(std::getline(stream, segment, ' '))
        argument_list.push_back(segment);

    int last_slash = path.find_last_of('/');
    int path_size = last_slash;

    std::wstring program_name = path.substr(path_size + 1, path.size());

    char executable_cstr[200];
    std::wcstombs(executable_cstr, path.c_str(), path.size());
    executable_cstr[path.size()] = '\0';

    char program_name_cstr[200];
    std::wcstombs(program_name_cstr, program_name.c_str(), program_name.size());
    program_name_cstr[program_name.size()] = '\0';
    char arg_1[] = "20";
    char arg_2[] = "3819";
    char *args[] = {program_name_cstr, arg_1, arg_2, NULL};
    //char *args[] = {NULL};
    //int arguments_size = arguments.size();
    //std::wcstombs(args_cstr, arguments.c_str(), arguments.size());
    //args_cstr[size] = '\0';

    int pid = fork();
    if(pid == 0)
        execvp(executable_cstr, args);
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