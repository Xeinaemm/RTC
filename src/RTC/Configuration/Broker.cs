using System.Diagnostics;
using System.Text;
using Xamarin.Forms;

namespace RTC.Configuration
{
    public class Broker
    {
        public void SendEvent(byte[] eventData, string receiver)
        {
            if (eventData == null) return;
            Debug.WriteLine(receiver);
            var payload = Encoding.UTF8.GetString(eventData);
            MessagingCenter.Send(this, "Data",payload);
        }
    }
}
