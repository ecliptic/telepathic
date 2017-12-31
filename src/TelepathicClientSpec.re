open Jest;

open Expect;

open Js.Option;

open TelepathicUtils;

external asSocket : Js.t('a) => TelepathicClient.Ws.t = "%identity";

describe(
  "Client",
  () => {
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
                try (TelepathicActions.decode(json) |> getExn) {
                | _exn => Js.Exn.raiseError("Unable to decode action")
                };
              expectMatches(
                ~message="Wrong key",
                ~expected="CLIENT_REGISTER",
                ~actual=TelepathicActions.key(action)
              );
              switch action {
              | ClientRegister(linkId) =>
                expectMatches(~message="Wrong linkId", ~expected=expectedLinkId, ~actual=linkId)
              | _ => Js.Exn.raiseError("Wrong action")
              }
            };
            let socket = Some(asSocket({"send": handleMessage, "on": () => ()}));
            let client = TelepathicClient.make(~socket, ~linkId=expectedLinkId, "test-url");
            let run = () => client |> TelepathicClient.register;
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
                try (Js.Json.parseExn(message) |> TelepathicActions.decode |> getExn) {
                | _exn => Js.Exn.raiseError("Unable to parse message")
                };
              expectMatches(
                ~message="Wrong key",
                ~expected="MESSAGE_SEND",
                ~actual=TelepathicActions.key(action)
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
            let socket = Some(asSocket({"send": handleMessage, "on": () => ()}));
            let client = TelepathicClient.make(~socket, ~linkId=expectedLinkId, "test-url");
            let run = () =>
              client |> TelepathicClient.sendMessage(~linkId=expectedLinkId, ~text=expectedText);
            expect(run) |> not_ |> toThrow
          }
        )
    )
  }
);
