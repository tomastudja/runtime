//
// RenderTeamPage.cs - Renders an HTML page with team member information from an XML file
//
// Author: Duncan Mak (duncan@ximian.com)
//
// (C) Copyright 2003, Ximian Inc.
//

using System;
using System.Collections;
using System.IO;
using System.Text;
using System.Xml;

class Write {

        static Contributor [] list;

        static void Main (string [] args)
        {
                if (args.Length != 2) {
                        Console.WriteLine ("write.exe <input.xml> <output.html>");
                        Environment.Exit (0);
                }

                string input = args [0];
                string output = args [1];
                XmlDocument document = new XmlDocument ();
                document.Load (input);

                XmlNodeList contributors = document.SelectNodes ("/contributors/contributor");
                list = new Contributor [contributors.Count];

                Page p = new Page ();

                int count = 0;
                foreach (XmlNode n in contributors) {
                        list [count] = new Contributor (n, p.Document);
                        count ++;
                }

                Array.Sort (list, new ContributorComparer ());

                int length = list.Length % 2 == 0 ? list.Length : list.Length + 1;

                int i = 0;
                while (i < length) {
                        try {
                                p.AddRow (list [i].RenderHtml (), list [i + 1].RenderHtml ());
                        } catch (IndexOutOfRangeException) {
                                p.AddRow (list [i].RenderHtml (), null);
                        }
                        i += 2;
                }

                p.Write (output);
        }
}

public class ContributorComparer : IComparer
{
        public int Compare (object x, object y)
        {
                return String.Compare (x as string , y as string);
        }
}

class Contributor {

        public Name name;
        public string email;
        public string image;
        public string location;
        public string organization;
        public string description;
        public string[] tasks;

        public XmlDocument document;

        public Contributor (XmlNode node, XmlDocument document)
        {

                name         = GetName (node);
                image        = GetField (node, "image");
                email        = GetField (node, "e-mail");
                location     = GetField (node, "location");
                organization = GetField (node, "organization");
                description  = GetField (node, "description");
                tasks        = GetTasks (node);

                this.document = document;
        }

        public override string ToString ()
        {
                return name.ToString ();
        }

        public static string GetField (XmlNode node, string selector)
        {
                XmlNode result = node.SelectSingleNode (selector);

                if (result == null)
                        return String.Empty;

                return result.InnerText;
        }

        public static Name GetName (XmlNode node)
        {
                string first_name = GetField (node, "name/first-name");
                string last_name = GetField (node, "name/last-name");

                return new Name (first_name, last_name);
        }

        public static string [] GetTasks (XmlNode node)
        {
                XmlNodeList nodes = node.SelectNodes ("tasks/task");

                string [] result = new string [nodes.Count];

                int i = 0;
                foreach (XmlNode n in nodes) {
                        result [i] = n.InnerText;

                        i++;
                }

                return result;
        }

        public XmlElement RenderHtml ()
        {
                XmlElement root = document.CreateElement ("TD");
                XmlElement table = document.CreateElement ("TABLE");
                table.SetAttribute ("width", "100%");
                XmlElement tr = document.CreateElement ("TR");
                XmlElement td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#c3cda7");
                td.SetAttribute ("valign", "top");
                tr.AppendChild (td);
                table.AppendChild (tr);
                root.AppendChild (table);

                XmlElement img = document.CreateElement ("IMG");
                img.SetAttribute ("align", "top");
                img.SetAttribute ("border", "0");
                img.SetAttribute ("height", "48");
                img.SetAttribute ("width", "48");
                img.SetAttribute ("src", image);
                td.AppendChild (img);

                td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#c3cda7");
                td.SetAttribute ("valign", "bottom");
                tr.AppendChild (td);

                td.AppendChild (name.ToXml (document));
                td.AppendChild (document.CreateElement ("BR"));
                td.AppendChild (RenderEmail ());

                tr = document.CreateElement ("TR");
                table.AppendChild (tr);
                td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#f5f8e4");
                td.SetAttribute ("valign", "top");
                tr.AppendChild (td);
                td.AppendChild (RenderLabel ("Location: "));

                td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#f5f8e4");
                td.SetAttribute ("valign", "top");
                tr.AppendChild (td);
                td.AppendChild (document.CreateTextNode (location));

                tr = document.CreateElement ("TR");
                table.AppendChild (tr);
                td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#f5f8e4");
                td.SetAttribute ("valign", "top");
                tr.AppendChild (td);
                td.AppendChild (RenderLabel ("Description: "));

                td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#f5f8e4");
                td.SetAttribute ("valign", "top");
                tr.AppendChild (td);
                td.AppendChild (document.CreateTextNode (description));

                tr = document.CreateElement ("TR");
                table.AppendChild (tr);
                td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#f5f8e4");
                td.SetAttribute ("valign", "top");
                tr.AppendChild (td);
                td.AppendChild (RenderLabel ("Tasks: "));

                td = document.CreateElement ("TD");
                td.SetAttribute ("bgcolor", "#f5f8e4");
                td.SetAttribute ("valign", "top");
                tr.AppendChild (td);
                td.AppendChild (RenderTasks ());

                return root;
        }

        public XmlNode RenderTasks ()
        {

                XmlElement element = document.CreateElement ("OL");
                element.SetAttribute ("type", "I");

                foreach (string task in tasks) {
                        XmlElement li = document.CreateElement ("LI");
                        li.AppendChild (document.CreateTextNode (task));
                        element.AppendChild (li);
                }

                return element;
        }

        public XmlNode RenderEmail ()
        {
                XmlElement a = document.CreateElement ("A");
                a.SetAttribute ("href", "mailto:" + email);
                XmlElement font = document.CreateElement ("FONT");
                font.SetAttribute ("size", "3");
                XmlText t = document.CreateTextNode (email);
                a.AppendChild (font);
                font.AppendChild (t);

                return a;
        }

        public XmlNode RenderLabel (string label)
        {
                string text = String.Format ("{0}: ", label);
                XmlElement element = document.CreateElement ("B");
                XmlText t = document.CreateTextNode (label );
                element.AppendChild (t);

                return element;
        }
}

class Page {

        XmlDocument document;
        XmlElement tbody;

        public Page ()
        {
                document = new XmlDocument ();
                XmlElement html = document.CreateElement ("HTML");
                document.AppendChild (html);

                XmlElement head = document.CreateElement ("HEAD");
                html.AppendChild (head);

                XmlElement body = document.CreateElement ("BODY");
                html.AppendChild (body);

                XmlElement table = document.CreateElement ("TABLE");
                body.AppendChild (table);

                tbody = document.CreateElement ("TBODY");
                table.AppendChild (tbody);
        }

        public XmlDocument Document {
                get { return document; }
        }

        public void AddRow (XmlNode left, XmlNode right)
        {
                if (left == null && right == null)
                        return;

                XmlElement tr = document.CreateElement ("TR");
                tbody.AppendChild (tr);
                tr.AppendChild (left);

                if (right == null)
                        tr.AppendChild (document.CreateElement ("TD"));
                else {
                        tr.SetAttribute ("valign", "top");
                        tr.AppendChild (right);
                }
        }

        public void Write (TextWriter text_writer)
        {
                XmlTextWriter writer = new XmlTextWriter (text_writer);
                writer.Formatting = Formatting.Indented;

                document.WriteContentTo (writer);

                writer.Flush ();
        }

        public void Write (string filename)
        {
                XmlTextWriter writer = new XmlTextWriter (filename, Encoding.Default);
                writer.Formatting = Formatting.Indented;

                document.WriteContentTo (writer);
                writer.Flush ();
        }
}


class Name {

        string first_name;
        string last_name;

        public Name (string a, string b)
        {
                this.first_name = a;
                this.last_name = b;
        }

        public override string ToString ()
        {
                if (first_name == null && last_name == null)
                        return String.Empty;

                return first_name + " " + last_name;
        }

        public XmlNode ToXml (XmlDocument document)
        {
                XmlElement element = document.CreateElement ("FONT");
                element.SetAttribute ("size", "3");
                XmlElement b = document.CreateElement ("B");
                XmlText t = document.CreateTextNode (ToString ());
                b.AppendChild (t);
                element.AppendChild (b);

                return element;
        }
}
