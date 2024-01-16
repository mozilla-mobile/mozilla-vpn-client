//
//  widgets.swift
//  widgets
//
//  Created by Matt Lichtenstein on 1/5/24.
//

import WidgetKit
import SwiftUI

struct Provider: TimelineProvider {
        
    func placeholder(in context: Context) -> SimpleEntry {
        SimpleEntry(date: Date(), emoji: "😀")
    }

    func getSnapshot(in context: Context, completion: @escaping (SimpleEntry) -> ()) {
        let entry = SimpleEntry(date: Date(), emoji: "😀")
        completion(entry)
    }

    func getTimeline(in context: Context, completion: @escaping (Timeline<Entry>) -> ()) {
        var entries: [SimpleEntry] = []

        // Generate a timeline consisting of five entries an hour apart, starting from the current date.
        let currentDate = Date()
        for hourOffset in 0 ..< 5 {
            let entryDate = Calendar.current.date(byAdding: .hour, value: hourOffset, to: currentDate)!
            let entry = SimpleEntry(date: entryDate, emoji: "😀")
            entries.append(entry)
        }

        let timeline = Timeline(entries: entries, policy: .atEnd)
        completion(timeline)
    }
}

struct SimpleEntry: TimelineEntry {
    let date: Date
    let emoji: String
}

struct widgetsEntryView : View {
    
    var impl = IOSWidgetsImpl()
    
    var entry: Provider.Entry
    
    @State private var isOn = false

    var body: some View {
        VStack {
            VStack {
                HStack(alignment: .top) {
                    Image("Off")
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                        .frame(width: 40, height: 40)
                    VStack(alignment: .leading) {
                        Text("VPN is off")
                            .bold()
                        Text(impl.retrieveCurrent())
                            .foregroundColor(Color(red: 109/255, green: 109/255, blue: 110/255))
                            .opacity(0.8)
                    }
                    Spacer()
                    Image("Logo")
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                        .frame(width: 24, height: 24)
                }
                .padding(16)
            }
            .background(Color.white)
            .padding(-16)
            
            Spacer()
            
            VStack(alignment: .leading) {
                HStack {
                    Text("Recent connections")
                        .font(.system(size: 11))
                        .bold()
                        .padding(.top, 16)
                    
                    Spacer()
                }
                Spacer()
                
                HStack(spacing: 16) {
                    VStack {
                        Text(impl.retrieveFirstRecent())
                            .font(.system(size: 11))
                    }
                    .frame(width: 108, height: 48)
                    .background(Color.white)
                    .clipShape(.rect(cornerRadius: 8))

                    VStack {
                        Text(impl.retrieveSecondRecent())
                            .font(.system(size: 11))
                    }
                    .frame(width: 108, height: 48)
                    .background(Color.white)
                    .clipShape(.rect(cornerRadius: 8))
                }
            }
            
        }
    }
}

struct widgets: Widget {
    let kind: String = "widgets"

    var body: some WidgetConfiguration {
        StaticConfiguration(kind: kind, provider: Provider()) { entry in
                widgetsEntryView(entry: entry)
                    .containerBackground(.fill.tertiary, for: .widget)
        }
        .configurationDisplayName("My Widget")
        .description("This is an example widget.")
    }
}

#Preview(as: .systemSmall) {
    widgets()
} timeline: {
    SimpleEntry(date: .now, emoji: "🤩")
    SimpleEntry(date: .now, emoji: "xxx")
}

#Preview(as: .systemMedium) {
    widgets()
} timeline: {
    SimpleEntry(date: .now, emoji: "🤩")
    SimpleEntry(date: .now, emoji: "xxx")
}

#Preview(as: .systemLarge) {
    widgets()
} timeline: {
    SimpleEntry(date: .now, emoji: "🤩")
    SimpleEntry(date: .now, emoji: "xxx")
}
