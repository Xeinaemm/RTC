using System;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Azure.EventHubs;
using Microsoft.Azure.WebJobs;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

using IoTHubTrigger = Microsoft.Azure.WebJobs.EventHubTriggerAttribute;

namespace Functions
{
    public static class Charlie
    {
        [FunctionName("Charlie")]
        public static async Task Run(
            [IoTHubTrigger("%eventHubConnectionPath%", Connection = "eventHubConnectionString")]
            EventData inputMessage,
            [EventHub("outeventhub", Connection = "outEventHubConnectionString")]
            IAsyncCollector<string> outputMessage,
            ILogger log)
        {
            var converter = new IsoDateTimeConverter {DateTimeFormat = "dd/MM/yy HH:mm:ss"};
            var myEventHubMessage = Encoding.UTF8.GetString(inputMessage.Body.Array);
            var input = JsonConvert.DeserializeObject<Input>(myEventHubMessage, converter);
            if (input == null)
            {
                log.LogError("Deserialization fault");
                return;
            }

            var output = new Output
            {
                DeviceId = input.DeviceId,
                Timestamp = input.Timestamp,
                Temperature = input.Temperature,
                Humidity = input.Humidity,
                X = input.X,
                Y = input.Y,
                Z = input.Z
            };

            //Consistency
            //var outputMessage = EventHubClient.CreateFromConnectionString(Environment.GetEnvironmentVariable("outEventHubConnectionString"));
            //var data = new EventData(Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(output))); <--- this is still availability
            //data.Properties.Add("0", JsonConvert.SerializeObject(output)); <--- but this allow you to pass data in consistent manner
            //await outputMessage.SendAsync(data);

            //Availability
            await outputMessage.AddAsync(JsonConvert.SerializeObject(output));
            log.LogInformation($"DeviceId={input.DeviceId}");
        }
    }

    internal class Input
    {
        public string DeviceId;
        public double Humidity;
        public double Temperature;
        public DateTime Timestamp;
        public double X;
        public double Y;
        public double Z;
    }

    internal class Output
    {
        public string DeviceId;
        public double Humidity;
        public double Temperature;
        public DateTime Timestamp;
        public double X;
        public double Y;
        public double Z;
    }
}