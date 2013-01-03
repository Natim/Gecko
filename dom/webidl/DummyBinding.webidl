/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Dummy bindings that we need to force generation of things that
// aren't actually referenced anywhere in IDL yet but are used in C++.

interface DummyInterface {
  readonly attribute OnErrorEventHandlerNonNull onErrorEventHandler;
  FilePropertyBag fileBag();
  RTCConfiguration rtcConfiguration();
  CFStateChangeEventDict cfstateChangeEvent();
  USSDReceivedEventDict ussdReceivedEvent();
  InspectorRGBTriple rgbTriple();
  Function getFunction();
};

interface DummyInterfaceWorkers {
  BlobPropertyBag blobBag();
};
