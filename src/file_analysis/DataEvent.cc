#include <string>

#include "DataEvent.h"
#include "EventRegistry.h"
#include "Event.h"
#include "util.h"

using namespace file_analysis;

DataEvent::DataEvent(Info* arg_info, EventHandlerPtr ce, EventHandlerPtr se)
    : Action(arg_info, BifEnum::FileAnalysis::ACTION_DATA_EVENT),
      chunk_event(ce), stream_event(se)
	{
	}

Action* DataEvent::Instantiate(const RecordVal* args, Info* info)
	{
	using BifType::Record::FileAnalysis::ActionArgs;

	const char* chunk_field = "chunk_event";
	const char* stream_field = "stream_event";
	int chunk_off = ActionArgs->FieldOffset(chunk_field);
	int stream_off = ActionArgs->FieldOffset(stream_field);

	Val* chunk_val = args->Lookup(chunk_off);
	Val* stream_val = args->Lookup(stream_off);

	if ( ! chunk_val && ! stream_val ) return 0;

	EventHandlerPtr chunk;
	EventHandlerPtr stream;

	if ( chunk_val )
		chunk = event_registry->Lookup(chunk_val->AsFunc()->GetID()->Name());

	if ( stream_val )
		stream = event_registry->Lookup(stream_val->AsFunc()->GetID()->Name());

	return new DataEvent(info, chunk, stream);
	}

bool DataEvent::DeliverChunk(const u_char* data, uint64 len, uint64 offset)
	{
	Action::DeliverChunk(data, len, offset);

	if ( ! chunk_event ) return true;

	val_list* args = new val_list;
	args->append(info->GetVal()->Ref());
	args->append(new StringVal(new BroString(data, len, 0)));
	args->append(new Val(offset, TYPE_COUNT));
	mgr.QueueEvent(chunk_event, args);

	return true;
	}

bool DataEvent::DeliverStream(const u_char* data, uint64 len)
	{
	Action::DeliverStream(data, len);

	if ( ! stream_event ) return true;

	val_list* args = new val_list;
	args->append(info->GetVal()->Ref());
	args->append(new StringVal(new BroString(data, len, 0)));
	mgr.QueueEvent(stream_event, args);

	return true;
	}
