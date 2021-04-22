// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#[cfg_attr(s2n_sys_internal_api, path = "internal.rs")]
mod api;

pub use api::*;

#[cfg(all(feature = "quic", not(s2n_sys_internal_api)))]
mod quic;

#[cfg(all(feature = "quic", not(s2n_sys_internal_api)))]
pub use quic::*;

#[cfg(test)]
mod tests;
