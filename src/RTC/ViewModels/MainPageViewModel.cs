using System.Windows.Input;
using Newtonsoft.Json;
using RTC.Configuration;
using Xamarin.Forms;

namespace RTC.ViewModels
{
    public class MainPageViewModel : BaseViewModel
    {
        private DeviceOutputModel _device;
        private string _interval;

        public MainPageViewModel()
        {
            MessagingCenter.Subscribe<Broker, string>(this, "Data", (sender, payload) =>
            {
                Device = JsonConvert.DeserializeObject<DeviceOutputModel>(payload);
            });

            RealTimeStateCommand = new Command(async () =>
            {
                await DeviceTwinHelper.SetDeviceTwin(Interval, "Interval");
            });
        }

        public ICommand RealTimeStateCommand { protected set; get; }

        public DeviceOutputModel Device
        {
            set
            {
                if (SetProperty(ref _device, value))
                    OnPropertyChanged(nameof(Device));
            }
            get => _device;
        }

        public string Interval
        {
            set
            {
                if (SetProperty(ref _interval, value))
                    OnPropertyChanged(nameof(Interval));
            }
            get => _interval;
        }
    }
}
