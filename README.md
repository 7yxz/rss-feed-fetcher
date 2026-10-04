## How to Run

*(Leave the URL empty to test with Hacker News by default)*

### Python (`rss.py`)
```bash
python3 rss.py [URL]
```

### Rust (`rss.rs`)
```bash
cargo run -- [URL]
```

### Java (`rss.java`)
```bash
java rss.java [URL]
```

### C (`rss.c`)
```bash
gcc rss.c -o rss $(pkg-config --cflags --libs libxml-2.0 libcurl)
./rss [URL]
```
