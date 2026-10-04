import java.io.ByteArrayInputStream;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.*;

public class FeedFetcher {

    public record Entry(String title, String link, String date) {}

    public static List<Entry> fetch(String feedUrl) throws Exception {
        HttpClient client = HttpClient.newBuilder()
                .followRedirects(HttpClient.Redirect.NORMAL)
                .connectTimeout(Duration.ofSeconds(10))
                .build();

        HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create(feedUrl))
                .header("User-Agent", "Mozilla/5.0")
                .GET().build();

        byte[] body = client.send(request, HttpResponse.BodyHandlers.ofByteArray()).body();

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(false);
        Document doc = factory.newDocumentBuilder().parse(new ByteArrayInputStream(body));

        List<Entry> results = new ArrayList<>();
        
        NodeList nodes = doc.getElementsByTagName("item");
        if (nodes.getLength() == 0) {
            nodes = doc.getElementsByTagName("entry");
        }

        for (int i = 0; i < nodes.getLength(); i++) {
            Element el = (Element) nodes.item(i);
            
            String title = getTagValue(el, "title");
            String link = getTagValue(el, "link");
            if (link.isEmpty()) {
                NodeList linkNodes = el.getElementsByTagName("link");
                if (linkNodes.getLength() > 0) {
                    Element linkEl = (Element) linkNodes.item(0);
                    link = linkEl.getAttribute("href");
                }
            }

            String date = getTagValue(el, "pubDate");
            if (date.isEmpty()) date = getTagValue(el, "updated");
            if (date.isEmpty()) date = getTagValue(el, "published");

            results.add(new Entry(title, link, date));
        }

        return results;
    }

    private static String getTagValue(Element el, String tag) {
        NodeList nl = el.getElementsByTagName(tag);
        return (nl.getLength() > 0 && nl.item(0).getFirstChild() != null)
                ? nl.item(0).getFirstChild().getNodeValue().trim() : "";
    }

    public static void main(String[] args) throws Exception {
        String url = (args.length > 0) ? args[0] : "https://news.ycombinator.com/rss";
        for (Entry entry : fetch(url)) {
            System.out.println("Title: " + entry.title());
            System.out.println("Link:  " + entry.link());
            System.out.println("Date:  " + entry.date());
            System.out.println("--------------------------------------------------");
        }
    }
}
