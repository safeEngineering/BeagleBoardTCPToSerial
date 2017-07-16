using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using TracerX;

namespace MonitorTwoCOM.Comminication
{ 
    public class Packet
    {
        #region Fields

        // Logging object
        private static readonly Logger log = Logger.GetLogger(typeof(Packet));
   
        public string PacketType { get; set; }           // type of packet
        public string PacketID { get; set; }             // id of packet
        public byte[] PacketData { get; set; }           // data of packet - (may be empty)
        public byte DataLen { get; set; }                // data length

        #endregion

        #region Constructors

        public Packet()
        {
            PacketType = "CMD";
            PacketID = "PING";
            DataLen = 0;
        }

        public Packet(string packetType, string packetID)
        {
            PacketType = packetType;
            PacketID = packetID;
            DataLen = 0;
        }

        public Packet(string packetType, string packetID, byte[] data, byte dataLen)
        {
            PacketType = packetType;
            PacketID = packetID;
            DataLen = dataLen;
            PacketData = new byte[dataLen];
            Array.Copy(data, PacketData, dataLen);
        }

        #endregion

        #region Static functions

        public static Packet Parse(string commandLine)
        {
            Packet result = null;
            string[] tokens;

            try
            {
                if (string.IsNullOrEmpty(commandLine))
                    return result;

                // remove '\r\n'
                commandLine = commandLine.Trim(new char[] { '\r', '\n' });
                // split string based on ',' character
                tokens = commandLine.Split(',');
                if (tokens == null || tokens.Length < 2)
                    return result;

                result = new Packet();
                result.PacketType = tokens[0].Trim();
                result.PacketID = tokens[1].Trim();
                if(tokens.Length == 3)
                {
                    result.DataLen = (byte)tokens[2].Length;
                    result.PacketData = Encoding.ASCII.GetBytes(tokens[2]);
                }
            }
            catch(Exception ex)
            {
                // write to log
                log.Error("Function Parse have thrown exception: ", ex.Message);
            }
            return result;
        }
        #endregion

        #region Methods

        /// <summary>
        /// Create ASCII string from packet
        /// </summary>
        public string BuildCommandString()
        {
            string result = null;

            try
            {
                result = string.Format("{0},{1}", PacketType, PacketID);
                if (PacketData != null && DataLen > 0)
                {
                    result += "," + Encoding.ASCII.GetString(PacketData, 0, DataLen);
                }
            }
            catch (Exception ex)
            {
                // write to log
                log.Error("Function BuildCommandString have thrown exception: ", ex.Message);
            }
            return result;
        }

        #endregion
    }
}
