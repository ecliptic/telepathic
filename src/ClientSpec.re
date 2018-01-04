open Jest;

open Expect;

open Js.Option;

open Utils;

external asSocket : Js.t('a) => WebSockets.WebSocket.t = "%identity";

describe(
  "Client",
  () => {
    open Client;
    describe(
      "register(client)",
      () =>
        test(
          "registers the linkId with the server",
          () => {
            let expectedLinkId = "test-uuid";
            let handleMessage = (message) => {
              let json =
                try (Js.Json.parseExn(message)) {
                | _exn => Js.Exn.raiseError("Unable to parse message")
                };
              let action =
                try (json |> Actions.Decode.action |> getExn) {
                | _exn => Js.Exn.raiseError("Unable to decode action")
                };
              expectMatches(
                ~message="Wrong key",
                ~expected="CLIENT_REGISTER",
                ~actual=Actions.key(action)
              );
              switch action {
              | ClientRegister(linkId) =>
                expectMatches(~message="Wrong linkId", ~expected=expectedLinkId, ~actual=linkId)
              | _ => Js.Exn.raiseError("Wrong action")
              }
            };
            let socket = Some(asSocket({"send": handleMessage, "addEventListener": (_) => ()}));
            let client = make(~socket, ~linkId=expectedLinkId, ~onMessage=(_) => (), "test-url");
            let run = () => client |> register;
            expect(run) |> not_ |> toThrow
          }
        )
    );
    describe(
      "sendMessage(~linkId, ~text)",
      () =>
        test(
          "uses the WebSocket client to send the message",
          () => {
            let expectedLinkId = "test-uuid";
            let expectedUserName = "Test Name";
            let expectedText = "Test message!";
            let handleMessage = (message) => {
              let action =
                try (message |> Js.Json.parseExn |> Actions.Decode.action |> getExn) {
                | _exn => Js.Exn.raiseError("Unable to parse message")
                };
              expectMatches(
                ~message="Wrong key",
                ~expected="MESSAGE_SEND",
                ~actual=Actions.key(action)
              );
              switch action {
              | MessageSend(linkId, userName, text) =>
                expectMatches(~message="Wrong linkId", ~expected=expectedLinkId, ~actual=linkId);
                expectMatches(
                  ~message="Wrong userName",
                  ~expected=expectedUserName,
                  ~actual=userName
                );
                expectMatches(~message="Wrong text", ~expected=expectedText, ~actual=text)
              | _ => Js.Exn.raiseError("Wrong action")
              }
            };
            let socket = Some(asSocket({"send": handleMessage, "addEventListener": (_) => ()}));
            let client = make(~socket, ~linkId=expectedLinkId, ~onMessage=(_) => (), "test-url");
            let run = () => client |> sendMessage(~linkId=expectedLinkId, ~text=expectedText);
            expect(run) |> not_ |> toThrow
          }
        )
    )
  }
);
