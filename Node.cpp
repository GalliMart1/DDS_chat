#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include "MessageTypeSupportImpl.h"

#include <iostream>
#include <string>

using namespace std;

// El Listener es una clase que "reacciona" a eventos de forma asíncrona.
class MessageDataReaderListenerImpl : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener> {
public:
    MessageDataReaderListenerImpl(int id) : my_id_(id) {}
    // Este método se dispara automáticamente cuando llega un mensaje nuevo a la red
    virtual void on_data_available(DDS::DataReader_ptr reader) {
        // Convertimos el lector genérico a uno específico para nuestro tipo 'Message'
        Chat::MessageDataReader_var msg_dr = Chat::MessageDataReader::_narrow(reader);
        if (CORBA::is_nil(msg_dr.in())) return;

        Chat::Message msg;
        DDS::SampleInfo si;
        // 'take_next_sample' extrae el mensaje de la cola del middleware
        if (msg_dr->take_next_sample(msg, si) == DDS::RETCODE_OK) {
            // Solo mostramos si el dato es válido y NO es nuestro propio mensaje
            if (si.valid_data && msg.sender_id != my_id_) {
                // FORMATO PERSONALIZADO AQUÍ
                cout << "\rID=" << msg.sender_id << ": " << msg.text << endl;
                cout << "> " << flush; // Vuelve a mostrar el prompt para escribir
            }
        }
    }

    // Métodos vacíos obligatorios
    virtual void on_requested_deadline_missed(DDS::DataReader_ptr, const DDS::RequestedDeadlineMissedStatus&) {}
    virtual void on_requested_incompatible_qos(DDS::DataReader_ptr, const DDS::RequestedIncompatibleQosStatus&) {}
    virtual void on_liveliness_changed(DDS::DataReader_ptr, const DDS::LivelinessChangedStatus&) {}
    virtual void on_subscription_matched(DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus&) {}
    virtual void on_sample_rejected(DDS::DataReader_ptr, const DDS::SampleRejectedStatus&) {}
    virtual void on_sample_lost(DDS::DataReader_ptr, const DDS::SampleLostStatus&) {}

private:
    int my_id_; // ID local para identificar al usuario
};

int main(int argc, char* argv[]) {
    try {
        // Inicializamos el participante
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);
        // Crear el Participante en el Dominio 42
        DDS::DomainParticipant_var participant = dpf->create_participant(42, PARTICIPANT_QOS_DEFAULT, 0, OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (!participant) {
            cerr << "Error creando el participante." << endl;
            return 1;
        }

        // Configuración de identidad del usuario
        int my_id = 0;
        cout << "Ingrese su ID de usuario: ";
        cin >> my_id;
        cin.ignore(); // Limpiar el buffer del enter para que no salte el getline después

        // Registro del Tipo de Dato definido en el IDL
        Chat::MessageTypeSupport_var ts = new Chat::MessageTypeSupportImpl;
        if (ts->register_type(participant, "") != DDS::RETCODE_OK) return 1;
        CORBA::String_var type_name = ts->get_type_name();

        // --- 1. CONFIGURACIÓN QoS DEL TOPIC ---
        // El Topic debe permitir durabilidad para que los nodos coincidan
        DDS::TopicQos topic_qos;
        participant->get_default_topic_qos(topic_qos);
        topic_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;

        // Crear el Topic "ChatTopic"
        DDS::Topic_var topic = participant->create_topic("ChatTopic", type_name, topic_qos, 0, OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        
        // --- 2. CONFIGURACIÓN QoS DEL PUBLICADOR (DataWriter) ---
        DDS::Publisher_var pub = participant->create_publisher(PUBLISHER_QOS_DEFAULT, 0, OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        DDS::DataWriterQos dw_qos;
        pub->get_default_datawriter_qos(dw_qos);
        dw_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS; // Guarda datos en memoria
        dw_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;      // Garantiza la entrega
        dw_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;            // Guarda los últimos N
        dw_qos.history.depth = 50;                                   // Cantidad de mensajes a recordar

        DDS::DataWriter_var dw = pub->create_datawriter(topic, dw_qos, 0, OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        // Crear Publicador y Escritor
        Chat::MessageDataWriter_var msg_dw = Chat::MessageDataWriter::_narrow(dw);

        // Crear Suscriptor y Lector con el Listener
        DDS::Subscriber_var sub = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, 0, OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        
        DDS::DataReaderQos dr_qos;
        sub->get_default_datareader_qos(dr_qos);
        dr_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS; // Pide datos históricos al unirse
        dr_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
        dr_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
        dr_qos.history.depth = 50;
        
        // El Listener se encarga de manejar los mensajes entrantes en un hilo separado
        DDS::DataReaderListener_var listener = new MessageDataReaderListenerImpl(my_id);
        DDS::DataReader_var dr = sub->create_datareader(topic, dr_qos, listener, OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        
        // Bucle principal de chat
        cout << "Chat Iniciado (Tu ID: " << my_id << ")" << endl;
        cout << "Escribí tu mensaje y dale Enter. ('exit' para salir)" << endl;
        
        string line;
        while (true) {
            cout << "> " << flush; // El prompt solo se imprime una vez al inicio del ciclo
            if (!getline(cin, line) || line == "exit") break;

            if (!line.empty()) {
                Chat::Message msg;
                msg.sender_id = my_id;
                msg.text = line.c_str();
                msg_dw->write(msg, DDS::HANDLE_NIL);
            }
        }

        // Limpieza al salir
        participant->delete_contained_entities();
        dpf->delete_participant(participant);
        TheServiceParticipant->shutdown();

    } catch (const CORBA::Exception& e) {
        cerr << "Error de CORBA: " << e << endl;
        return 1;
    }
    return 0;
}
