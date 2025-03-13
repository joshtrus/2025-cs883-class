1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The client waits for a special EOF marker to know when it’s received the full response. Since TCP can split messages into several packets, the client  uses a loop with recv() to read until it sees that marker. Other techniques are prefixing the message with its length so that the client knows exactly how many bytes to expect.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

Because TCP sends a continuous stream of bytes without clear breaks, the shell protocol needs to add its own marker (or use a length header) so the client knows when one message ends. Without that, messages could mix or split up and make parsing hard.

3. Describe the general differences between stateful and stateless protocols.

Stateful protocols remember past interactions (session data) across multiple requests, while stateless protocols treat every request as new. This means stateful protocols can provide context, but stateless ones are simpler and easier to scale.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

Even though UDP doesn’t guarantee that messages get there or in order, it’s useful for applications like live video or gaming where speed is more important than perfect delivery and where some data loss is acceptable.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The operating system gives us the Sockets API, which lets programs create sockets and use them to send and receive data over a network without having to worry about all the low-level details.






