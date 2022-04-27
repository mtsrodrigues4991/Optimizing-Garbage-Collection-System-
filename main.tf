#Smart Bin project
######################################################################################################################################################

terraform {
  required_providers {
    azurerm = {                                            
      source  = "hashicorp/azurerm"
      version = "~> 2.66"
    }
  }
  required_version = ">= 0.14.9"
}

provider "azurerm" {
  features {}
}

######################################################################################################################################################

resource "azurerm_resource_group" "rg" {                   
  name     = "rg-smart-bin" 
  location = "eastus"
  tags = {
    owner = "Mateus Rodrigues"                                
    environment = "Development"                                  
    team = "IoTDev"                                              
  }
}


######################################################################################################################################################

resource "azurerm_storage_account" "salakebronze" {
  name                     = "datalakebzeiot"                       
  resource_group_name      = azurerm_resource_group.rg.name
  location                 = azurerm_resource_group.rg.location
  account_tier             = "Standard"
  account_replication_type = "LRS"
  account_kind             = "StorageV2"
  is_hns_enabled           = "true"
}

resource "azurerm_storage_container" "contbronze" {                      #cria um conteiner, nada mais é que uma pasta dentro do datalake
  name                  = "original"
  storage_account_name  = azurerm_storage_account.salakebronze.name
  container_access_type = "private"
}

######################################################################################################################################################

resource "azurerm_storage_account" "converted" {
  name                     = "datalakeconverted"                        #service account para bronze
  resource_group_name      = azurerm_resource_group.rg.name
  location                 = azurerm_resource_group.rg.location
  account_tier             = "Standard"
  account_replication_type = "LRS"
  account_kind             = "StorageV2"
  is_hns_enabled           = "true"
}

resource "azurerm_storage_container" "conv" {                      #cria um conteiner, nada mais é que uma pasta dentro do datalake
  name                  = "converted"
  storage_account_name  = azurerm_storage_account.converted.name
  container_access_type = "private"
}


######################################################################################################################################################

resource "azurerm_iothub" "iothublorawan" {                              #cria e define um IoTHub
  name                = "iot-hub-lorawan-az-1"
  resource_group_name = azurerm_resource_group.rg.name
  location            = azurerm_resource_group.rg.location

  sku {                                                                 #define o plano do IoTHub
    name     = "S1"
    capacity = "1"
  }
  
  endpoint {                                              #dentro do IoTHub é criado um endpoint, isso permite direcionar as mensagens que chegam ao IoTHub para o service account
    type                       = "AzureIotHub.StorageContainer"
    connection_string          = azurerm_storage_account.salakebronze.primary_blob_connection_string
    name                       = "exportbronze"
    batch_frequency_in_seconds = 60                          #frequencia q deve ser enviada a informação, nesse caso 60segundos
    max_chunk_size_in_bytes    = 10485760                  #a mensagem é enviada a 60s ou quando a mensagem tem este tamanho
    container_name             = azurerm_storage_container.contbronze.name
    encoding                   = "JSON"                      #formato que a mensagem deve ser enviada
    file_name_format           = "{iothub}/{partition}/{YYYY}/{MM}/{DD}/{HH}/{mm}.json"   #em qual pasta vai ser armazenada
  }

  route {	                                                #define a rota 
    name           = "exportdatalake"
    source         = "DeviceMessages"
    condition      = "true"
    endpoint_names = ["exportbronze"]
    enabled        = true
  }

}


######################################################################################################################################################
## IoTHub For Paessler bitdecoder

resource "azurerm_iothub_dps" "iot_hub_paessler" {
  name                = "lorawanpaessler"
  resource_group_name = azurerm_resource_group.rg.name
  location            = azurerm_resource_group.rg.location
  allocation_policy   = "Hashed"

  sku {
    name     = "S1"
    capacity = "1"
  }
  
  tags = {
      name = "paessler"
      environment =  "iot_hub_paessler"
  }
}


######################################################################################################################################################

resource "azurerm_cosmosdb_account" "realtime-db" {             #cria uma conta no CosmoDB, antes de criar a base de dados é requerido essa conta/recurso
  name                = "db-lorawan-iot-message"
  location            = azurerm_resource_group.rg.location
  resource_group_name = azurerm_resource_group.rg.name
  offer_type          = "Standard"
  kind                = "GlobalDocumentDB"

  consistency_policy {
    consistency_level       = "BoundedStaleness"
    max_interval_in_seconds = 10
    max_staleness_prefix    = 200
  }

  geo_location {
    location          = "eastus"
    failover_priority = 0
  }
}

resource "azurerm_cosmosdb_sql_database" "example" {    #define a base de dados, neste caso um sql no cosmodb
  name                = "lorawan_Message_IoT"
  resource_group_name = azurerm_cosmosdb_account.realtime-db.resource_group_name
  account_name        = azurerm_cosmosdb_account.realtime-db.name
  throughput          = 400
}

resource "azurerm_cosmosdb_sql_container" "example" {  #após definir a base de dados é definido o container/pasta que armazena a base de dados
  name                  = "Converted_Data"
  resource_group_name   = azurerm_cosmosdb_account.realtime-db.resource_group_name
  account_name          = azurerm_cosmosdb_account.realtime-db.name
  database_name         = azurerm_cosmosdb_sql_database.example.name
  partition_key_path    = "/device"
  partition_key_version = 1
  throughput            = 400   #limite de mensagens armazenadas

}



######################################################################################################################################################


resource "azurerm_stream_analytics_job" "stream" {
  name                                     = "stream-lorawan-paessler"
  resource_group_name                      = azurerm_resource_group.rg.name
  location                                 = azurerm_resource_group.rg.location
  compatibility_level                      = "1.1"
  data_locale                              = "en-GB"
  events_late_arrival_max_delay_in_seconds = 60
  events_out_of_order_max_delay_in_seconds = 50
  events_out_of_order_policy               = "Adjust"
  output_error_policy                      = "Drop"
  streaming_units                          = 3

  tags = {
    environment = "StreamAnalytics"
  }

  transformation_query = <<QUERY
    SELECT *
    INTO datalakeconverted
    FROM lorawan_paessler
QUERY

}