using System;
using System.Diagnostics;
using ppatierno.AzureSBLite.Messaging;
using RTC.Configuration;

namespace RTC.UWP.Configuration
{
    public static class CharlieActivityListener
    {
        private static EventHubClient Client { get; } = EventHubClient.CreateFromConnectionString(
            Constants.CharlieActivityEventHubEndpoint,
            Constants.CharlieActivityEventHubName);

        private static EventHubConsumerGroup ConsumerGroup { get; } = Client.GetDefaultConsumerGroup();

        private static EventHubReceiver Receiver { get; } =
            ConsumerGroup.CreateReceiver("0", DateTime.Now.ToUniversalTime());

        private static EventHubReceiver ReceiverSecond { get; } =
            ConsumerGroup.CreateReceiver("1", DateTime.Now.ToUniversalTime());

        private static Broker Broker { get; } = new Broker();

        public static void StartListening()
        {
            while (true)
            {
                try
                {
                    var eventData = Receiver.Receive();
                    Broker.SendEvent(eventData.GetBytes(), Receiver.PartitionId);
                    //Broker.SendEvent(eventData.Properties["0"] as byte[], Receiver.PartitionId);
                    //Debug.WriteLine("<--------------------------------------->");
                    Broker.SendEvent(ReceiverSecond.Receive().GetBytes(), ReceiverSecond.PartitionId);
                }
                catch
                {
                    // ignored
                }
            }
        }
    }
}