import time

class Metronome(object):
    def __init__(self, fps):
        self.delay = 1./fps

    def start(self):
        self.last = time.time()

    def sync(self):
        now = time.time()
        if self.delay > now-self.last:
            time.sleep(self.delay - (now-self.last))
        self.last = max(now, self.last+self.delay)

