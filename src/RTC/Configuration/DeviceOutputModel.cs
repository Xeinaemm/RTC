using System;

namespace RTC.Configuration
{
    public class DeviceOutputModel
    {
        public string DeviceId { get; set; }
        public double Humidity { get; set; }
        public double Temperature { get; set; }
        public DateTime Timestamp { get; set; }
        public double X { get; set; }
        public double Y { get; set; }
        public double Z { get; set; }
    }
}