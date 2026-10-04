import sys
import urllib.request
import xml.etree.ElementTree as ET

def fetch_feed(url: str):
    headers = {"User-Agent": "Mozilla/5.0"}
    req = urllib.request.Request(url, headers=headers)
    
    with urllib.request.urlopen(req, timeout=15) as response:
        xml_data = response.read()

    root = ET.fromstring(xml_data)
    
    for elem in root.iter():
        if "}" in elem.tag:
            elem.tag = elem.tag.split("}", 1)[1]

    entries = root.findall(".//item") or root.findall(".//entry")
    items = []

    for entry in entries:
        title = entry.findtext("title", default="No Title").strip()
        
        link_elem = entry.find("link")
        link = ""
        if link_elem is not None:
            link = link_elem.attrib.get("href") or link_elem.text or ""
        
        date = (
            entry.findtext("pubDate") or 
            entry.findtext("published") or 
            entry.findtext("updated") or 
            "No Date"
        ).strip()

        items.append({"title": title, "link": link.strip(), "date": date})
    
    return items

if __name__ == "__main__":
    feed_url = sys.argv[1] if len(sys.argv) > 1 else "https://news.ycombinator.com/rss"
    for item in fetch_feed(feed_url):
        print(f"Title: {item['title']}")
        print(f"Link:  {item['link']}")
        print(f"Date:  {item['date']}")
        print("-" * 50)
