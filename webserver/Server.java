import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.Buffer;

public class Server {
    private static final int APP_MAX_BUFFER = 1024;
    private static final int PORT = 8080;

    public static void main(String[] args) {
        int server_fd, client_fd;
        try{
            ServerSocket serverSocket = new ServerSocket(PORT);
           // serverSocket.bind(new InetSocketAddress("0.0.0.0",PORT));
            System.out.println("Server started. Listening on port " + PORT);

            while(true){
                System.out.println("Waiting for client connection");
                Socket clientSocket = serverSocket.accept();
                System.out.println("Client request : " + clientSocket.toString());

                BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                PrintWriter out = new PrintWriter(clientSocket.getOutputStream(),true);

                char[] buffer = new char[APP_MAX_BUFFER];
                int bytesRead = in.read(buffer,0,APP_MAX_BUFFER);
                if(bytesRead == -1){
                    // Edge case to handle that client has terminated it's connection.
                }else {
                    String request = new String(buffer, 0, bytesRead);
                    System.out.println("Client request");
                    System.out.println(request);

                    String httpResponse = "HTTP/1.1 200 OK\n" +
                            "Content-Type: text/plain\n" +
                            "Content-Length: 13\n\n" +
                            "Hello world!\n";
                    out.write(httpResponse);
                    out.flush();

                    clientSocket.close();
                }

            }

        }catch (IOException exception){

        }
    }
}
