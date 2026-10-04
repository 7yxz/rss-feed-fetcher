use std::env;
use std::error::Error;

pub struct FeedEntry {
    pub title: String,
    pub link: String,
    pub published: String,
}

pub fn fetch_feed(url: &str) -> Result<Vec<FeedEntry>, Box<dyn Error>> {
    let client = reqwest::blocking::Client::builder()
        .user_agent("Mozilla/5.0")
        .build()?;

    let bytes = client.get(url).send()?.bytes()?;
    let feed = feed_rs::parser::parse(&bytes[..])?;

    let entries = feed.entries.into_iter().map(|entry| {
        FeedEntry {
            title: entry.title.map(|t| t.content).unwrap_or_else(|| "No Title".into()),
            link: entry.links.first().map(|l| l.href.clone()).unwrap_or_default(),
            published: entry.published.or(entry.updated)
                .map(|d| d.to_rfc2822())
                .unwrap_or_else(|| "No Date".into()),
        }
    }).collect();

    Ok(entries)
}

fn main() -> Result<(), Box<dyn Error>> {
    let args: Vec<String> = env::args().collect();
    let url = args.get(1).map(|s| s.as_str()).unwrap_or("https://news.ycombinator.com/rss");

    for entry in fetch_feed(url)? {
        println!("Title: {}", entry.title);
        println!("Link:  {}", entry.link);
        println!("Date:  {}", entry.published);
        println!("{}", "-".repeat(50));
    }

    Ok(())
}
