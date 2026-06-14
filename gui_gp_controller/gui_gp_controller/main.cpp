#include "main.hpp"
#include "OscMessage.hpp"
#include "Defines.hpp"

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_COMMAND(wxID_ANY, ROTARY_KNOB_UPDATED, MyFrame::OnRotary_Knob_Event)
wxEND_EVENT_TABLE()

bool MyApp::OnInit()
{

	MyFrame* frame;
	if (wxApp::argc != 5) {
		printf("arguments have to be: udp port in, udp port out, midi_port_in, midi_port_out, EXIT\n");
		exit(0);
	}
	else {
		std::string arg_1(wxApp::argv[1]);
		std::string arg_2(wxApp::argv[2]);
		std::string arg_3(wxApp::argv[3]);
		std::string arg_4(wxApp::argv[4]);
		frame = new MyFrame(arg_1, arg_2, arg_3, arg_4);
	}
	frame->Show();
	return true;
}

MyFrame::MyFrame(std::string udp_input_port, std::string udp_output_port, std::string midi_port_in, std::string midi_port_out)
	: wxFrame(nullptr, wxID_ANY, "General Purpose Controller (gp_controller)", wxDefaultPosition, wxSize(800, 400))
{
	main_layout = new wxBoxSizer(wxVERTICAL);
	button_layout = new wxBoxSizer(wxVERTICAL);
	//label_layout = new wxBoxSizer(wxHORIZONTAL);
	fader_layout = new wxBoxSizer(wxVERTICAL);

	plugin_name = new wxStaticText(this, wxID_ANY, "plugin_name", wxDefaultPosition, wxSize(120, 30));

	bank_down = new wxButton(this, wxID_ANY, "bank down");
	bank_up = new wxButton(this, wxID_ANY, "bank up");
	plugin_down = new wxButton(this, wxID_ANY, "plugin down");
	plugin_up = new wxButton(this, wxID_ANY, "plugin up");
	bank_down->Bind(wxEVT_BUTTON, &MyFrame::bank_down_function, this);
	bank_up->Bind(wxEVT_BUTTON, &MyFrame::bank_up_function, this);
	plugin_down->Bind(wxEVT_BUTTON, &MyFrame::plugin_down_function, this);
	plugin_up->Bind(wxEVT_BUTTON, &MyFrame::plugin_up_function, this);
	button_layout->Add(bank_down);
	button_layout->Add(bank_up);
	button_layout->Add(plugin_down);
	button_layout->Add(plugin_up);
	
	for (int i = 0; i < 16; i++) {
		//controller[i].name = new wxStaticText(this, wxID_ANY, "test", wxDefaultPosition, wxSize(100, 30));
		controller[i].rotary_knob = new rotary_knob_modul(i, this, wxID_ANY, wxDefaultPosition, wxSize(100, 50), wxBORDER_RAISED);
		controller[i].index = i + 1;
		controller[i].handler = (wxEvtHandler*)this;
		//label_layout->Add(controller[i].name);
		fader_layout->Add(controller[i].rotary_knob);
		//controller[i].fader->Bind(wxEVT_SLIDER, &instance::OnSlider, &controller[i]);
	}
	main_layout->Add(plugin_name);
	main_layout->Add(button_layout);
	//main_layout->Add(label_layout);
	main_layout->Add(fader_layout);
	this->SetSizerAndFit(main_layout);
	this->osc_sender_receiver = new OscSenderReceiver("127.0.0.1", std::stoi(udp_input_port), std::stoi(udp_output_port));
	this->receive_thread = new wxOscReceiveThread(this, this->osc_sender_receiver);
	receive_thread->Run();
	Bind(wxEVT_THREAD, &MyFrame::OnThreadUpdate, this);

	midi_receiver = new MidiSenderReceiver(std::stoi(midi_port_in), std::stoi(midi_port_out));
	midi_receive_thread = new wxMidiReceiveThread(this, this->midi_receiver);
	midi_receive_thread->Run();

	this->plugin_multiplexer = new plugin_multiplexer_c();

	OscMessage setup_msg("/set_surface");
	setup_msg.PushInt(1);
	setup_msg.PushInt((1 << AudioTracks) | (1 << MidiTracks));
	setup_msg.PushInt(1 << ExtraSelectOnlyFeedback);
	setup_msg.PushInt(0b1000);

	this->osc_sender_receiver->send_data(setup_msg);

}
//these could also all be handled by the thread handler...
void MyFrame::bank_up_function(wxCommandEvent& event)
{
	if (bank < (BANK_SIZE - 1))
		bank++;
	this->update_controller();
}
void MyFrame::bank_down_function(wxCommandEvent& event)
{
	if (bank > 0)
		bank--;
	this->update_controller();
}

void MyFrame::plugin_up_function(wxCommandEvent& event)
{
	OscMessage msg = OscMessage("/select/plugin");
	msg.PushFloat(1);
	this->osc_sender_receiver->send_data(msg);
}

void MyFrame::plugin_down_function(wxCommandEvent& event)
{
	//OscMessage msg = OscMessage("/strip/plugin/list");
	//msg.PushInt(1);
	OscMessage msg = OscMessage("/select/plugin");
	msg.PushFloat(-1);
	this->osc_sender_receiver->send_data(msg);
}

void MyFrame::update_controller()
{
	this->reset_controller();
	for (int i = 0; i < CONTROLLER_SIZE; i++) {
		controller[i].rotary_knob->set_text(this->selected_plugin[i + (CONTROLLER_SIZE * bank) + 1].name);
		controller[i].rotary_knob->set_value(this->selected_plugin[i + (CONTROLLER_SIZE * bank) + 1].value);
		struct rgb_colour colour = plugin_multiplexer->get_knob_colour(i + (CONTROLLER_SIZE * bank));
		controller[i].rotary_knob->set_knob_colour(wxColour(colour.r, colour.g, colour.b));
	}
}

void instance::OnSlider(wxCommandEvent& event)
{
	OscMessage osc_message("/wxSlider");
	//thread_message message;
	
	osc_message.PushInt(this->index + (bank * CONTROLLER_SIZE));
	int value = event.GetInt();
	float value_float = float(value) / 1000.0;
	osc_message.PushFloat(value_float);

	//this should be automatically done every time something is pushed...
	osc_message.FormatOscMessage(); 

	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	event_1.SetPayload(osc_message);
	wxQueueEvent(handler, event_1.Clone());
}


void MyFrame::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	OscMessage osc_message = event.GetPayload<OscMessage>();

	if (!osc_message.GetAddress().compare("/quadrature_rotary_encoder")) {
		int encoder_id = osc_message.get_int(0)  + (bank * CONTROLLER_SIZE);
		short increment = (osc_message.get_int(1) << 2);
		increment /= 4;

		if (encoder_id == 1)
			increment *= 30;
		
		previous_value[encoder_id] += increment;
		if (previous_value[encoder_id] < 0)
			previous_value[encoder_id] = 0;
		if (previous_value[encoder_id] > 1800)
			previous_value[encoder_id] = 1800;
		float result = 0;
		result = previous_value[encoder_id] / 1800.0;
		if (result > 1)
			result = 1;
		//this->controller[encoder_id].rotary_knob->set_value(result);

	int plugin_parameter_id_routed = this->plugin_multiplexer->get_controller_to_plugin(encoder_id);
	OscMessage osc_message("/select/plugin/parameter");
	
	osc_message.PushInt(plugin_parameter_id_routed);
	osc_message.PushFloat(result);

	//this should be automatically done every time something is pushed...
	osc_message.FormatOscMessage(); 
	this->osc_sender_receiver->send_data(osc_message);
	}
	if (!osc_message.GetAddress().compare("/select/plugin/parameter/name")) {
		int plugin_parameter_id = osc_message.get_int(0);
		std::string plugin_parameter_name = osc_message.get_string(1);

		if (plugin_parameter_id >= this->plugin_multiplexer->get_plugin_size())
			return;	

		int plugin_parameter_id_routed = this->plugin_multiplexer->get_plugin_to_controller(plugin_parameter_id);
		this->selected_plugin[plugin_parameter_id_routed].name = plugin_parameter_name;

		if (plugin_parameter_id_routed >= MAX_PLUGIN_PARAMETERS)
			return;

		if (plugin_parameter_id_routed <= 0)
			return;	

		if ((plugin_parameter_id_routed / CONTROLLER_SIZE) == bank) {
			//this->controller[(plugin_parameter_id_routed - 1) % CONTROLLER_SIZE].name->SetLabel(plugin_parameter_name);
			this->controller[(plugin_parameter_id_routed - 1) % CONTROLLER_SIZE].rotary_knob->set_text(plugin_parameter_name);
		}
	}
	else if (!osc_message.GetAddress().compare("/select/plugin/parameter")) {
		int plugin_parameter_id = osc_message.get_int(0);
		float plugin_parameter_value = osc_message.initialize_type_list().at(1) == 'f' ? osc_message.get_float(1) : osc_message.get_double(1);
			
		if (plugin_parameter_id >= this->plugin_multiplexer->get_plugin_size())
			return;	

		int plugin_parameter_id_routed = this->plugin_multiplexer->get_plugin_to_controller(plugin_parameter_id);
		this->selected_plugin[plugin_parameter_id_routed].value = plugin_parameter_value;
		
		if (plugin_parameter_id_routed <= 0)
			return;

		this->previous_value[(plugin_parameter_id_routed) - 1] = plugin_parameter_value * 1800;

		if ((plugin_parameter_id_routed / CONTROLLER_SIZE) == bank) {
			this->controller[(plugin_parameter_id_routed - 1) % CONTROLLER_SIZE].rotary_knob->set_value(plugin_parameter_value);
			//I update here the knob colour at the same time of the plugin_parameter
			struct rgb_colour colour = plugin_multiplexer->get_knob_colour(plugin_parameter_id_routed - 1);
			controller[(plugin_parameter_id_routed - 1) % CONTROLLER_SIZE].rotary_knob->set_knob_colour(wxColour(colour.r, colour.g, colour.b));
		}
	}
	else if (!osc_message.GetAddress().compare("/wxSlider")) {
		int index = osc_message.get_int(0);
		float value = (float)osc_message.get_float(1);
		int plugin_parameter_id_routed = this->plugin_multiplexer->get_controller_to_plugin(index);

		OscMessage osc_message_to_ardour("/select/plugin/parameter");
		osc_message_to_ardour.PushInt(plugin_parameter_id_routed);
		osc_message_to_ardour.PushFloat(value);
		int size = 0;
		osc_message.GetBytes(size);
		this->osc_sender_receiver->send_data(osc_message_to_ardour);
	}
	else if (!osc_message.GetAddress().compare("/select/plugin/name")) {
		std::string plugin_name = osc_message.get_string(0);
		if (plugin_name.size() == 1)
			return;
		this->clear_cache();
		this->plugin_multiplexer->setup(plugin_name);

		this->plugin_name->SetLabel(plugin_name);
		if (this->selected_plugin_name == plugin_name) //everything is already setup...
			return;
		this->selected_plugin_name = plugin_name;
		this->reset_controller();
	}
	return;
}

//thats guis data
void MyFrame::reset_controller()
{
	for (int i = 0; i < CONTROLLER_SIZE; i++) {
		this->controller[i].rotary_knob->set_value(0.5);
		this->controller[i].rotary_knob->set_text(" ");
		this->controller[i].rotary_knob->set_knob_colour(wxColour(255, 255, 255));
	}
}

//thats local data
void MyFrame::clear_cache()
{
	for (int i = 0; i < MAX_PLUGIN_PARAMETERS; i++){
		this->selected_plugin[i].name = std::string(" ");
		this->selected_plugin[i].value = 0;
}
}



void MyFrame::OnRotary_Knob_Event(wxCommandEvent& event)
{
	int data = event.GetInt();
	int id = event.GetId() + (bank * CONTROLLER_SIZE);
	int plugin_parameter_id_routed = this->plugin_multiplexer->get_controller_to_plugin(id);
	OscMessage osc_message("/select/plugin/parameter");
	
	osc_message.PushInt(plugin_parameter_id_routed);
	int value = data;
	float value_float = (float(value) + 225.0) / 270.0;
	osc_message.PushFloat(value_float);

	//this should be automatically done every time something is pushed...
	osc_message.FormatOscMessage(); 
	this->osc_sender_receiver->send_data(osc_message);
	//wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	//event_1.SetPayload(osc_message);
	//wxQueueEvent(handler, event_1.Clone());

}
