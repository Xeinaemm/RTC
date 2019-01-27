using System;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;

namespace RTC.Configuration
{
    public static class DeviceTwinHelper
    {
        private static RegistryManager RegistryManager { get; } =
            RegistryManager.CreateFromConnectionString(Constants.DeviceMessagingConnectionString);

        public static async Task SetDeviceTwin(string state, string key)
        {
            try
            {
                var twin = await GetTwin();
                if (twin == null) return;
                
                if (key.Equals("Interval", StringComparison.CurrentCultureIgnoreCase))
                    await UpdateIntervalState(state, twin);
            }
            catch (Exception e)
            {
                Debug.WriteLine(e);
            }
        }

        private static async Task<Twin> GetTwin()
        {
            var twins = await RegistryManager.CreateQuery("SELECT * from devices", 1).GetNextAsTwinAsync();
            return twins.FirstOrDefault();
        }

        private static async Task UpdateIntervalState(string state, Twin twin)
        {
            if (int.TryParse(state, out var interval))
            {
                var patch = new
                {
                    properties = new
                    {
                        desired = new
                        {
                            interval
                        }
                    }
                };
                await RegistryManager.UpdateTwinAsync(twin.DeviceId, JsonConvert.SerializeObject(patch), twin.ETag);
            }
        }
    }
}