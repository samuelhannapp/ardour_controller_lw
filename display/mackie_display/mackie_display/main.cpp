#include "main.hpp"
#include "OscMessage.hpp"
#include <vector>
#include <fstream>
#include "Defines.hpp"
#include <filesystem>


bool MyApp::OnInit()
{
	MyFrame* frame;
	if(wxApp::argc != 3){
		printf("arguments have to be: udp port in, udp port out\n");
		frame = new MyFrame(std::string(), std::string());
	}
	else {
		std::string arg_1(wxApp::argv[1]);
		std::string arg_2(wxApp::argv[2]);
		frame = new MyFrame(arg_1, arg_2);
	}
		
	frame->Show();
    return true;
}

MyFrame::MyFrame(std::string udp_port_in, std::string udp_port_out)
    : wxFrame(nullptr, wxID_ANY, "Base Controller (Mackie Control) Display", wxDefaultPosition, wxSize(800, 400))
{
    wxMenu* menuConnections = new wxMenu;
    menuConnections->Append(ID_Hello, "&display format", "setup ip address for controller");
    menuConnections->Append(ID_OTHER, "&other...\tCtrl+H", "adjust display size to controller");
    menuConnections->AppendSeparator();
    menuConnections->Append(wxID_EXIT);

    wxMenu* menuDisplayFormat = new wxMenu;
    menuDisplayFormat->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuConnections, "display format");
    menuBar->Append(menuDisplayFormat, "about");

    SetMenuBar(menuBar);

    //CreateStatusBar();
    //SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

	channel_layout = new wxBoxSizer(wxHORIZONTAL);
	main_layout = new  wxBoxSizer(wxVERTICAL);

	spacers.push_back(channel_layout->AddSpacer(SPACE_SIZE_SIDE));

	channel_not_selected_color = wxColor(210, 210, 210);
	channel_selected_color = wxColor(230, 230, 230);
	for (int i = 0; i < CHANNEL_COUNT; i++) {
		panel[i] = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
		panel[i]->SetMinSize(wxSize(100, 100));
		panel[i]->SetBackgroundColour(channel_not_selected_color);
		channel_name_panel[i] = new wxPanel(panel[i], wxID_ANY, wxPoint(10, 10) , wxSize(80, 50), wxBORDER_SUNKEN);
		channel_name_panel[i]->SetMinSize(wxSize(80, 50));
		plugin_parameter_name_panel[i] = new wxPanel(panel[i], wxID_ANY, wxPoint(10, 10) , wxSize(80, CHANNEL_NAME_HEIGHT), wxBORDER_SUNKEN);
		plugin_parameter_name_panel[i]->SetMinSize(wxSize(80, CHANNEL_NAME_HEIGHT));
		knob_name_panel[i] = new wxPanel(panel[i], wxID_ANY, wxPoint(10, 10) , wxSize(80, CHANNEL_NAME_HEIGHT), wxBORDER_SUNKEN); 
		knob_name_panel[i]->SetMinSize(wxSize(80, CHANNEL_NAME_HEIGHT));
		send_name_panel[i] = new wxPanel(panel[i], wxID_ANY, wxPoint(10, 10) , wxSize(80, CHANNEL_NAME_HEIGHT), wxBORDER_SUNKEN); 
		send_name_panel[i]->SetMinSize(wxSize(80, CHANNEL_NAME_HEIGHT));
		channel_name[i] = new wxStaticText(channel_name_panel[i], wxID_ANY, " ", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
		plugin_parameter_name[i] = new wxStaticText(plugin_parameter_name_panel[i], wxID_ANY, " ", wxDefaultPosition, wxSize(80, CHANNEL_NAME_HEIGHT), wxALIGN_CENTER);
		knob_name[i] = new wxStaticText(knob_name_panel[i], wxID_ANY, " ", wxDefaultPosition, wxSize(80, 50), wxALIGN_CENTER);
		send_name[i] = new wxStaticText(send_name_panel[i], wxID_ANY, " ", wxDefaultPosition, wxSize(80, 50), wxALIGN_CENTER);
		plugin_names_panel[i] = new wxPanel(panel[i], wxID_ANY, wxPoint(10, 10), wxSize(160, 50), wxBORDER_SUNKEN);
		plugin_names_panel[i]->SetMinSize(wxSize(80, CHANNEL_NAME_HEIGHT));
		plugin_names[i] = new wxStaticText(plugin_names_panel[i], wxID_ANY, " ", wxDefaultPosition, wxSize(80, 50), wxALIGN_LEFT);
		wxFont font_2 = channel_name[i]->GetFont();
		font_2.SetPointSize(8);
		plugin_names[i]->SetFont(font_2);
		wxFont font = channel_name[i]->GetFont();
		font.SetWeight(wxFONTWEIGHT_BOLD);
		font.SetPointSize(10);
		channel_name[i]->SetFont(font);
		channel_name[i]->SetBackgroundColour(channel_selected_color);
		channel_layout->Add(panel[i], 0, wxEXPAND);
		if(i != (CHANNEL_COUNT - 1))
			spacers.push_back(channel_layout->AddSpacer(SPACE_SIZE));
	}
	//spacers.push_back(channel_layout->AddSpacer(SPACE_SIZE_SIDE));

	knob_name[0]->SetLabel("Show BUSES");
	knob_name[1]->SetLabel("Show VCAS");
	knob_name[2]->SetLabel("Page Down");
	knob_name[3]->SetLabel("Page Up");
	knob_name[4]->SetLabel("Mode");
	knob_name[5]->SetLabel("Plugin Down");
	knob_name[6]->SetLabel("Plugin Up");
	knob_name[7]->SetLabel("Spill");

	top_spacer = main_layout->AddSpacer(SPACE_SIZE);
	main_layout->Add(channel_layout);
	main_layout->AddSpacer(SPACE_SIZE);
	
	this->SetSizer(main_layout);
	
	if (!udp_port_in.size())
		udp_port_in = std::string("100");
	if (!udp_port_out.size())
		udp_port_out = std::string("101");
	this->osc_sender_receiver = new OscSenderReceiver("127.0.0.1", std::stoi(udp_port_in), std::stoi(udp_port_out));
	Bind(wxEVT_SIZE, &MyFrame::OnSize, this);
	thread = new wxOscReceiveThread(this, osc_sender_receiver);
	thread->Run();
	Bind(wxEVT_THREAD, &MyFrame::OnThreadUpdate, this);
	#ifdef __linux__
	file_name_scaling = std::string("/home/samuel/Documents/display_scaling.txt");
	#endif
	#ifdef _WIN64
	file_name_scaling = std::string("C:\\Users\\Samuel\\Documents\\display_scaling.txt");
	#endif
	std::ifstream file(file_name_scaling);
	if (file.is_open()) {
		std::string line;
		OscMessage message("/wxSliderUpdate");
		std::getline(file, line);
		space_size_side = std::stoi(line) / 14;
		std::getline(file, line);
		left_spacer_size = std::stoi(line) / 3;
		std::getline(file, line);
		panel_offset = std::stoi(line);
		wxSize original_size = this->GetSize();
		wxSize temp_size(600, 600);
		this->SetSize(temp_size);
		this->SetSize(original_size);
	}
}

void MyFrame::OnSize(wxSizeEvent& event)
{
	this->adjust_window(event.GetSize());	
	this->Update();
	event.Skip();
	return;
}

void MyFrame::adjust_window(wxSize size)
{
	wxSize panel_size;
	panel_size = wxSize(size.GetWidth() / 8 - SPACE_SIZE - (space_size_side), size.GetHeight() - MENU_SIZE - SPACE_SIZE - panel_offset + 20);
	spacers[0]->SetMinSize(wxSize(left_spacer_size, 0));
	top_spacer->SetMinSize(wxSize(0, panel_offset));
	
	wxSize panel_height(panel_size.GetWidth() - SPACE_SIZE * 2, CHANNEL_NAME_HEIGHT);
	wxSize plugin_names_height(panel_size.GetWidth() - SPACE_SIZE * 2, 120);
	wxSize channel_name_panel_height(panel_size.GetWidth() - SPACE_SIZE * 2, CHANNEL_NAME_HEIGHT + 30);
	for (int i = 0; i < CHANNEL_COUNT; i++) {
		panel[i]->SetMinSize(panel_size);
		plugin_names_panel[i]->SetSize(plugin_names_height);
		channel_name_panel[i]->SetSize(channel_name_panel_height);
		knob_name_panel[i]->SetSize(panel_height);
		plugin_parameter_name_panel[i]->SetSize(panel_height);
		send_name_panel[i]->SetSize(panel_height);
		plugin_names_panel[i]->SetPosition(wxPoint(SPACE_SIZE, size.GetHeight() - CHANNEL_NAME_HEIGHT - MENU_SIZE - SPACE_SIZE * 2 - panel_offset - 180 - 30));
		channel_name_panel[i]->SetPosition(wxPoint(SPACE_SIZE, size.GetHeight() - CHANNEL_NAME_HEIGHT - MENU_SIZE - SPACE_SIZE * 2 - panel_offset - 30));
		plugin_parameter_name_panel[i]->SetPosition(wxPoint(SPACE_SIZE, size.GetHeight() - CHANNEL_NAME_HEIGHT - MENU_SIZE - SPACE_SIZE * 2 - panel_offset - 20 - 30));
		send_name_panel[i]->SetPosition(wxPoint(SPACE_SIZE, size.GetHeight() - CHANNEL_NAME_HEIGHT - MENU_SIZE - SPACE_SIZE * 2 - panel_offset - 40 - 30));
		knob_name_panel[i]->SetPosition(wxPoint(SPACE_SIZE, size.GetHeight() - CHANNEL_NAME_HEIGHT - MENU_SIZE - SPACE_SIZE * 2 - panel_offset + 20));
		//channel_name[i]->SetSize(channel_name_size);
	}
}

void MyFrame::OnSlider(wxCommandEvent& event)
{
	space_size_side = event.GetInt() / 14;
	wxSize original_size = this->GetSize();
	wxSize temp_size(600, 600);
	this->SetSize(temp_size);
	this->SetSize(original_size);
}

void MyFrame::OnSlider_2(wxCommandEvent& event)
{
	left_spacer_size = event.GetInt() / 3;
	wxSize original_size = this->GetSize();
	wxSize temp_size(600, 600);
	this->SetSize(temp_size);
	this->SetSize(original_size);
}

void MyFrame::OnSlider_3(wxCommandEvent& event)
{
	panel_offset = event.GetInt();
	wxSize original_size = this->GetSize();
	wxSize temp_size(600, 600);
	this->SetSize(temp_size);
	this->SetSize(original_size);
}

void MyFrame::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	OscMessage message = event.GetPayload<OscMessage>();
	if(!message.GetAddress().compare("/strip/name"))
	{
		//when the message /strip/name arrives the plugin_parameter window
		//can be deleted...
		int index = message.get_int(0) - 1;
		this->channel_name[index]->SetLabel(message.get_string(1));
		this->channel_name[index]->SetSize(channel_name_panel[index]->GetSize());
	}
	if(!message.GetAddress().compare("/select/send_name"))
	{
		int index = message.get_int(0) - 1;
		this->send_name[index]->SetLabel(message.get_string(1));
	}
	else if(!message.GetAddress().compare("/select/plugin/name")){
		std::string plugin_name = message.get_string(0);
		if(plugin_name.size() == 1)
			return;
		//knob_name[this->selected_channel]->SetLabel(plugin_name);
	}
	if(!message.GetAddress().compare("/select/plugin/parameter/name"))
	{
		int index = message.get_int(0) - 1;
		this->plugin_parameter_name[index]->SetLabel(message.get_string(1));
		this->plugin_parameter_name[index]->SetSize(plugin_parameter_name_panel[index]->GetSize());
	}
	if(!message.GetAddress().compare("/strip/select"))
	{
		int index = message.get_int(0);
		int active = message.get_float(1);
		if(active){
			this->panel[index - 1]->SetBackgroundColour(channel_selected_color);
			this->selected_channel = index - 1;
		}
		else
			this->panel[index - 1]->SetBackgroundColour(channel_not_selected_color);
		this->panel[index - 1]->Refresh();
	}
	if(!message.GetAddress().compare("/wxSliderUpdate"))
	{
		space_size_side	= message.get_int(0)  / 14;
		left_spacer_size = message.get_int(1)  / 3;
		panel_offset = message.get_int(2);
		wxSize original_size = this->GetSize();
		wxSize temp_size(600, 600);
		this->SetSize(temp_size);
		this->SetSize(original_size);
	}
	if(!message.GetAddress().compare("/strip/plugin/list"))
	{
		//local_strip_data.selected_strip.plugin_list.clear();
		//write_to_itm(std::to_string(message.GetTypeList().size()));
		int plugin_quantity = (message.GetTypeList().size() - 1) / 3;
		std::string type_list = message.GetTypeList();
		std::string plugin_list;
		for(int i = 0; i < plugin_quantity; i++){
			plugin_list.append(message.get_string(i + 2 + i * 2));
			plugin_list.push_back('\n');
		}
		plugin_names[this->selected_channel]->SetLabel(plugin_list);
		for(int i = 0; i < CHANNEL_COUNT; i++)
			if(i != this->selected_channel)
				plugin_names[i]->SetLabel(" ");
	}
	if(!message.GetAddress().compare("/base_controller/mode_switch")){
		
		switch(message.get_int(0)){
		case channel_mode::PanMode:
			for(int i = 0; i < CHANNEL_COUNT; i++)
				plugin_parameter_name[i]->SetLabel(" ");

			for(int i = 0; i < CHANNEL_COUNT; i++)
				send_name[i]->SetLabel(" ");

			for (int i = 0; i < CHANNEL_COUNT; i++) {
				plugin_parameter_name_panel[i]->SetBackgroundColour(channel_not_selected_color); 
				plugin_parameter_name_panel[i]->Refresh();
			}

			for (int i = 0; i < CHANNEL_COUNT; i++) {
				send_name_panel[i]->SetBackgroundColour(channel_not_selected_color); 
				send_name_panel[i]->Refresh();
			}

			for (int i = 0; i < CHANNEL_COUNT; i++) {
				channel_name_panel[i]->SetBackgroundColour(channel_selected_color); 
				channel_name_panel[i]->Refresh();
			}
			break; 
		case channel_mode::SendMode:
			for(int i = 0; i < CHANNEL_COUNT; i++)
				plugin_parameter_name[i]->SetLabel(" ");
			for (int i = 0; i < CHANNEL_COUNT; i++) {
				plugin_parameter_name_panel[i]->SetBackgroundColour(channel_not_selected_color); 
				plugin_parameter_name_panel[i]->Refresh();
			}
			for (int i = 0; i < CHANNEL_COUNT; i++) {
				send_name_panel[i]->SetBackgroundColour(channel_selected_color); 
				send_name_panel[i]->Refresh();
			}
			break; 
		case channel_mode::PluginMode:
			for(int i = 0; i < CHANNEL_COUNT; i++)
				send_name[i]->SetLabel(" ");
			for (int i = 0; i < CHANNEL_COUNT; i++) {
				send_name_panel[i]->SetBackgroundColour(channel_not_selected_color); 
				send_name_panel[i]->Refresh();
			}
			for (int i = 0; i < CHANNEL_COUNT; i++) {
				plugin_parameter_name_panel[i]->SetBackgroundColour(channel_selected_color); 
				plugin_parameter_name_panel[i]->Refresh();
			}
			break;
		}
	}
	
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

void MyFrame::OnHello(wxCommandEvent& event)
{
	WindowScalerFrame *scaler_window = new WindowScalerFrame(this, wxID_ANY, "scaler", wxPoint(100, 100), wxSize(300, 300));
	scaler_window->Show();
	event.Skip();
	return;
}

WindowScalerFrame::WindowScalerFrame(MyFrame *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(parent, id, title, pos, size)
{
	this->main_window = parent;
	slider_1 = new wxSlider(this, wxID_ANY, SPACE_SIZE, SPACE_SIZE, 800);
	slider_2 = new wxSlider(this, wxID_ANY, SPACE_SIZE, SPACE_SIZE, 800);
	slider_3 = new wxSlider(this, wxID_ANY, SPACE_SIZE, SPACE_SIZE, 800);
	wxBoxSizer* slider_layout = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* label_layout = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* main_layout = new wxBoxSizer(wxVERTICAL);

	wxButton* save_button = new wxButton(this, wxID_ANY, "save setting");
	wxButton* recover_button = new wxButton(this, wxID_ANY, "recover setting");

	slider_value_1 = new wxStaticText(this, wxID_ANY, "value");
	slider_value_2 = new wxStaticText(this, wxID_ANY, "value");
	slider_value_3 = new wxStaticText(this, wxID_ANY, "value");
	
	slider_layout->Add(slider_1);
	slider_layout->Add(slider_2);
	slider_layout->Add(slider_3);

	label_layout->Add(slider_value_1);
	label_layout->Add(slider_value_2);
	label_layout->Add(slider_value_3);
	label_layout->Add(save_button);
	label_layout->Add(recover_button);

	main_layout->Add(slider_layout);
	main_layout->Add(label_layout);

	this->SetSizer(main_layout);

	slider_1->Bind(wxEVT_SLIDER, &MyFrame::OnSlider, parent);
	slider_2->Bind(wxEVT_SLIDER, &MyFrame::OnSlider_2, parent);
	slider_3->Bind(wxEVT_SLIDER, &MyFrame::OnSlider_3, parent);

	slider_1->Bind(wxEVT_SLIDER, &WindowScalerFrame::OnSlider_1, this);
	slider_2->Bind(wxEVT_SLIDER, &WindowScalerFrame::OnSlider_2, this);
	slider_3->Bind(wxEVT_SLIDER, &WindowScalerFrame::OnSlider_3, this);

	save_button->Bind(wxEVT_BUTTON, &WindowScalerFrame::SaveScaling, this);
	recover_button->Bind(wxEVT_BUTTON, &WindowScalerFrame::OpenScaling, this);
}

void WindowScalerFrame::SaveScaling(wxCommandEvent& event)
{
	std::filesystem::remove(file_name_scaling);
	std::ofstream file(file_name_scaling);
	if (file.is_open()) {
		std::string s_value_1(slider_value_1->GetLabel().ToStdString());
		std::string s_value_2(slider_value_2->GetLabel().ToStdString());
		std::string s_value_3(slider_value_3->GetLabel().ToStdString());
		std::isdigit(s_value_1[0]) ? file << s_value_1 : file << 400;
		file << '\n';
		std::isdigit(s_value_2[0]) ? file << s_value_2 : file << 400;
		file << '\n';
		std::isdigit(s_value_3[0]) ? file << s_value_3 : file << 400;
		file << '\n';
		file.close();
	}
}

void WindowScalerFrame::OpenScaling(wxCommandEvent& event)
{
	std::ifstream file(file_name_scaling);
	std::string line;
	OscMessage message("/wxSliderUpdate");
	std::getline(file, line);
	slider_1->SetValue(std::stoi(line));
	slider_value_1->SetLabel(line);
	message.PushInt(std::stoi(line));
	std::getline(file, line);
	slider_2->SetValue(std::stoi(line));
	slider_value_2->SetLabel(line);
	message.PushInt(std::stoi(line));
	std::getline(file, line);
	slider_3->SetValue(std::stoi(line));
	slider_value_3->SetLabel(line);
	file.close();
	message.PushInt(std::stoi(line));
	wxThreadEvent thread_event = wxThreadEvent();
	thread_event.SetPayload(message);
	wxQueueEvent(this->main_window, thread_event.Clone());
}

void WindowScalerFrame::OnSlider_1(wxCommandEvent& event)
{
	this->slider_value_1->SetLabel(std::to_string(event.GetInt()));
	event.Skip();
}

void WindowScalerFrame::OnSlider_2(wxCommandEvent& event)
{
	this->slider_value_2->SetLabel(std::to_string(event.GetInt()));
	event.Skip();
}

void WindowScalerFrame::OnSlider_3(wxCommandEvent& event)
{
	this->slider_value_3->SetLabel(std::to_string(event.GetInt()));
	event.Skip();
}