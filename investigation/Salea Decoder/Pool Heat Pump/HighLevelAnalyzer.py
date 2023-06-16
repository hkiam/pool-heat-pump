# High Level Analyzer
# For more information and documentation, please go to https://support.saleae.com/extensions/high-level-analyzer-extensions
import functools
from saleae.analyzers import HighLevelAnalyzer, AnalyzerFrame, StringSetting, NumberSetting, ChoicesSetting

def reverse_bit(num):
    result = 0
    for i in range(8):
        result = (result << 1) + (num & 1)
        num >>= 1
    return result

leading_pulse_length = 9000
leading_space_length = 4500
pulse_length = 1000
span = 500

# High level analyzers must subclass the HighLevelAnalyzer class.
class Hla(HighLevelAnalyzer):

    # An optional list of types this analyzer produces, providing a way to customize the way frames are displayed in Logic 2.
    result_types = {
        'mytype': {
            'format': '{{data.result}}'
        }, 
        'bytedata': {
            'format': '{{data.result}}'
        }
    }

    def __init__(self):
        '''
        Initialize HLA.

        Settings can be accessed using the same name used above.
        '''
        self.reset()

    def reset(self):
            self.state = "wait_leading_pulse"
            self.bitbuffer = 1
            self.bytestart = None
            self.bytedata = []

    def decode(self, frame: AnalyzerFrame):
        '''
        Process a frame from the input analyzer, and optionally return a single `AnalyzerFrame` or a list of `AnalyzerFrame`s.

        The type and data values in `frame` will depend on the input analyzer.
        '''
        value = frame.data['data']
        this_frame_size = int(float(frame.end_time - frame.start_time) * 10e5)
        if this_frame_size >= leading_pulse_length:
            self.reset()

        if self.state == "wait_leading_pulse":
            if abs(this_frame_size-leading_pulse_length)<span and value == 0 :
                self.state = "wait_leading_space"
                return AnalyzerFrame('mytype', frame.start_time, frame.end_time, {
                    'result': 'Leading Pulse'
                })
            else:
                self.reset()
                return
        elif self.state == "wait_leading_space" and value == 1:
            if abs(this_frame_size-leading_space_length)<span:
                self.state = "wait_pulse"
                return AnalyzerFrame('mytype', frame.start_time, frame.end_time, {
                    'result': 'Leading Space'
                })
            else:
                self.reset()
                return
        elif self.state == 'wait_pulse' and value == 0: 
            if abs(this_frame_size-pulse_length)<span:
                self.state = "wait_bit"
                if self.bytestart is None:
                    self.bytestart = frame.start_time
                return 
            else:
                self.reset()
                return
            
        elif self.state == 'wait_bit':
            if value != 1:
                self.reset()
                return
            
            bit = 0
            if this_frame_size > 2000:
                bit = 1
            self.bitbuffer = (self.bitbuffer<<1) | bit

            self.state = "wait_pulse"
            if self.bitbuffer & 0b100000000:
                byte = reverse_bit(self.bitbuffer&0xFF)
                self.bytedata.append(byte)
                start_time = self.bytestart
                self.bitbuffer = 1
                self.bytestart = None
                if len(self.bytedata) == 10:
                    if self.bytedata[0] & 0x20 == 0x20:
                        self.bytedata = list(map(lambda a: a ^ 0xFF, self.bytedata))
                    d1 = ' '.join('{:02x}'.format(x) for x in self.bytedata)
                    cs =  self.bytedata[9] == functools.reduce(lambda a, b: (a+b)&0xFF, self.bytedata[1:9])
                    self.bytedata = []
                    return AnalyzerFrame('bytedata', start_time, frame.end_time, {
                    'result': '%02X' % byte,
                    'data' : d1,
                    'CS' : cs
                    })
        
                return AnalyzerFrame('bytedata', start_time, frame.end_time, {
                    'result': '%02X' % byte
                })            
            return
