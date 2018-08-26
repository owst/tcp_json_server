require 'socket'

socket_and_buffers = Array.new(2) { [TCPSocket.new('localhost', 2048), ""] }

loop do
  socket_and_buffers.each do |socket, buffer|
    if buffer.end_with?("\0")
      puts buffer
      buffer.replace("")
    end

    begin
      buffer << socket.read_nonblock(1)
    rescue IO::WaitReadable
    rescue EOFError
      puts "Socket with source port #{socket.addr[1]} EOF!"
      socket.close
    end
  end

  socket_and_buffers = socket_and_buffers.reject { |s, _| s.closed? }

  break if socket_and_buffers.empty?

  IO.select(socket_and_buffers.map(&:first))
end
