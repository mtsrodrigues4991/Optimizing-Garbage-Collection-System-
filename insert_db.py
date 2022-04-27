from azure.cosmos import CosmosClient
from azure.eventhub import EventHubConsumerClient
import json

#Credentials Event ubs
CONNECTION_STR = ""
EVENTHUB_NAME = ''

#Credentials CosmosDB
URL = ''
KEY = ''

client = CosmosClient(URL, credential = KEY)
database_name = ''
database = client.get_database_client(database_name)
container_name = ''
container = database.get_container_client(container_name)

def on_event(partition_context, event):
    # Put your code here.
    # If the operation is i/o intensive, multi-thread will have better performance.
    print("Received event from partition: {}.".format(partition_context.partition_id))
    event_obj = json.loads(next(event.body).decode('UTF-8'))
    print(event_obj)

    container.upsert_item({            
            'device' : event_obj['device'],
            'time' : event_obj['time'],
            'data' : event_obj['data'],
            'seqNumber' : event_obj['seqNumber'],            
            'deviceTypeId' : event_obj['deviceTypeId']
        }
    )

if __name__ == '__main__':
    consumer_client = EventHubConsumerClient.from_connection_string(
        conn_str = CONNECTION_STR,
        consumer_group = '$Default',
        eventhub_name = EVENTHUB_NAME,
    )

    consumer_client.receive(on_event = on_event,  starting_position = "-1")