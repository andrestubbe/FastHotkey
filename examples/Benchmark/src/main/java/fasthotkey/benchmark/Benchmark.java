package fasthotkey.benchmark;

import fasthotkey.FastHotkey;
import fasthotkey.KeyCodes;
import fasthotkey.ModifierKeys;
import org.openjdk.jmh.annotations.*;

import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.MILLISECONDS)
@State(Scope.Benchmark)
@Warmup(iterations = 2, time = 1, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 3, time = 1, timeUnit = TimeUnit.SECONDS)
@Fork(1)
public class Benchmark {

    @Setup
    public void setup() throws Exception {
        FastHotkey.loadLibrary();
    }

    @org.openjdk.jmh.annotations.Benchmark
    public boolean benchmarkRegisterAndUnregister() {
        boolean reg = FastHotkey.register(100, ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_ALT, KeyCodes.VK_F12, id -> {});
        boolean unreg = FastHotkey.unregister(100);
        return reg && unreg;
    }
}
