using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ChessServer
{
    public class Player
    {
        public Socket socket;
        public int player;

        public Player(Socket socket, int player = 0) {
            this.socket = socket;
            this.player = player;
        }
    }

    internal class Program
    {
        static List<Player> players = new List<Player>();
        static object playersLock = new Object();

        static int port = 4466;
        static void Main(string[] args)
        {
            //create listener
            System.Net.Sockets.TcpListener listener = new System.Net.Sockets.TcpListener(new IPEndPoint(IPAddress.Any, port));

            //wait for new connections
            listener.Start(2);

            Console.WriteLine("server started on port {0:D}", port);
            while(true) {
                Socket newCli = listener.AcceptSocket();
                Player newPlayer = new Player(newCli);
                lock(playersLock) {
                    players.Add(newPlayer);
                }

                //listen on new player
                Thread playerThread = new Thread(() => playerListen(newPlayer));
                playerThread.Start();
            }
        }

        static void playerListen(Player player) {
            StreamReader reader = new StreamReader(new NetworkStream(player.socket));
            //get player
            if (reader.ReadLine() == "w") {
                player.player = 1;
            } else {
                player.player = -1;
            }

            Console.WriteLine("player {0:D} joined", player.player);
            //listen to new moves
            while (true) {
                string? recvLine = null;
                try
                {
                    recvLine = reader.ReadLine();
                    
                }
                catch (System.Exception)
                {
                    break;
                }
                //close connection if received line is empty
                if (recvLine == null)
                    break;
                if (recvLine == "")
                    break;
                
                //forward move to opponent
                lock(playersLock) {
                    for (int i = 0; i < players.Count; i++) {
                        if (players[i].player != player.player) {
                            Console.WriteLine("forwarding {0} to {1}", recvLine, players[i].player);
                            try
                            {
                                //send to all other clients in case connection got closed
                                //note this doesn't make sure all bytes are sent. BUT we send so few bytes it should be fine
                                players[i].socket.Send(Encoding.ASCII.GetBytes(recvLine + "\n"));
                            }
                            catch (SocketException ex)
                            {
                                //send failed
                                Console.WriteLine("send failed: {0}\n{1}", ex.Message, ex.StackTrace);
                            }
                        }
                    }
                }
            }

            Console.WriteLine("player {0:D} left", player.player);

            try
            {
                //make sure connection is closed and remove player
                lock (playersLock) {
                    players.Remove(player);
                }
                player.socket.Close();
            }
            catch (System.Exception)
            {

            }
        }
    }
}